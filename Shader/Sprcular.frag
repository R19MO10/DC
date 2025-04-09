/**
* @file Sprcular.frag
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition;	// ワールド座標
layout(location=1) in vec2 inTexcoord;	// テクスチャ座標
layout(location=2) in vec3 inNormal;	// 法線ベクトル
layout(location=3) in vec4 inTangent;	// タンジェントベクトル
layout(location=4) in vec3 inShadowTexcoord; // シャドウテクスチャ座標

// 出力する色データ
out vec4 outColor;

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;	// 基本色テクスチャ
layout(binding=1) uniform sampler2D texEmission;// 発光色テクスチャ
layout(binding=2) uniform sampler2D texNormal;	// 法線テクスチャ
layout(binding=3) uniform sampler2DShadow texShadow; // 影用の深度テクスチャ

// プログラムからの入力
layout(location=1) uniform mat4 cameraTransformMatrix;  // カメラの座標変換行列

layout(location=99)  uniform float alphaCutoff; // フラグメントを破棄する境界値
layout(location=100) uniform vec4 color;		// 物体の色
layout(location=101) uniform int useTexEmission;// エミッションテクスチャの有無
layout(location=102) uniform vec4 emission;		// 物体の発光色
layout(location=103) uniform vec2 roughnessAndMetallic;
#define Roughness	(roughnessAndMetallic.x) // 物体表面の粗さ(0.0～1.0)
#define Metallic	(roughnessAndMetallic.y) // 金属かどうか(0=非金属 1=金属)

layout(location=107) uniform vec3 ambientLight;	// 環境光

// 平行光源
struct DirectionalLight
{
	vec3 color;     // 色と明るさ
	vec3 direction; // 光の向き
};
layout(location=108) uniform DirectionalLight directionalLight;

// ライト
struct Light
{
	vec3 color;		// 色と明るさ
	float falloffAngle; // スポットライトの減衰開始角度(Spot)
	
	vec3 position;	// 位置
	float radius;	// ライトが届く最大半径

	vec3 direction; // ライトの向き(Spot)
	float coneAngle;// スポットライトが照らす角度(Spot)
};
// ライト用SSBO
layout(std430, binding=0) readonly buffer LightDataBlock
{
	int lightCount;	// ライトの数
	int dummy[3];	// 上記のlightCountと合わせて64バイトにするための埋め合わせ
	Light lightList[];	// すべてのライトデータ
};


const float PI = 3.14159265;		// π
const float Gamma = 2.2;			// ガンマ値(2.2はCRTモニターのガンマ値)


/**
* 法線を計算する
*/
vec3 ComputeWorldNormal()
{
	vec3 normal = texture(texNormal, inTexcoord).rgb;
	
	/* 値が小さすぎる場合、法線テクスチャが設定されていないと判断して
		頂点法線を返す(RGBが全て 0 はありえないため) */
	if (dot(normal, normal) <= 0.0001) {
		return normalize(inNormal);
	}
	
	// 8bit値であることを考慮しつつ0～1を-1～+1に変換(128を0とみなす)
	normal = normal * (255.0 / 127.0) - (128.0 / 127.0);

	/* 法線テクスチャの値をワールド空間に変換
		(バイタンジェントの正しい方向をinTangent.wに入れておく) */
	vec3 bitangent = inTangent.w * cross(inNormal, inTangent.xyz);
	
	return normalize(
		inTangent.xyz * normal.x + bitangent * normal.y + inNormal * normal.z);
}

/**
* 法線分布項
* 
* @param normal		法線
* @param halfway	入射光線ベクトルと視線ベクトルの中間ベクトル
* @param roughness	物体表面の粗さ
*
* @return 視点から見えるすべてのマイクロファセット法線の平均値
*/
float NormalDistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
	// 光線を視線方向に反射する微小平面の比率を求める
	/* 表面法線の向きがハーフベクトルと近いほど、
		光線を視線方向に反射する微小平面が多くなる */
	float NdotH = max(dot(normal, halfway), 0);
	float NdotH2 = NdotH * NdotH;

	// dot(N, H)が影響する比率を求める
	float r2 = roughness * roughness;
	float r4 = r2 * r2;
	float denom = (NdotH2 * (r4 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return r4 / denom;
}

/**
* 幾何減衰項
* 
* @param NdotL		法線と入射光線の光源方向ベクトルの内積
* @param NdotV		法線と視線ベクトルの内積
* @param roughness	物体表面の粗さ
*
* @return 光線が視点に到達する比率
*/
float GeometricAttenuationSchlick(float NdotL, float NdotV, float roughness)
{
	// 物体表面の凹凸の度合い
	float k = (roughness + 1) * (roughness + 1) * 0.125;

	// 光源方向から見た幾何学的減衰項を計算
	/* 入射ベクトルに対する微小平面の見えやすさ */
	float g0 = NdotL / (NdotL * (1 - k) + k);

	// 視点方向から見た幾何学的減衰項を計算
	/* 視線ベクトルに対する微小平面の見えやすさ */
	float g1 = NdotV / (NdotV * (1 - k) + k);

	return g0 * g1;
}

/**
* フレネル項
* 
* @param fresnel0	角度0のフレネル値
* @param VdotH		入射光線とカメラの視線の中間方向ベクトルの内積
*
* @return 光の反射と透過の比率ベクトル(フレネル項)
*/
vec3 FresnelSchlick(vec3 fresnel0, float VdotH)
{
	// シュリックの近似式
	/* 入射ベクトルと法線のなす角が0°に近いほど反射が弱くなり、
		90°に近いほど反射が強くなる */
	float vh1 = 1 - VdotH;
	float vh2 = vh1 * vh1;
	return fresnel0 + (1 - fresnel0) * (vh2 * vh2 * vh1);
}


const float MinCosTheta = 0.000001;	// cosθの最小値

/**
* 鏡面反射BRDF
*
* @param normal			法線
* @param halfway		入射光線ベクトルと視線ベクトルの中間ベクトル
* @param roughness		物体表面の粗さ
* @param cameraVector	カメラの視線方向ベクトル
* @param NdotL			法線と入射光線の光源方向ベクトルの内積
* @param fresnel		フレネル項
*
* @return 鏡面反射の強さベクトル
*/
vec3 SpecularBRDF(
  vec3 normal, vec3 halfway, float roughness, vec3 cameraVector, float NdotL, vec3 fresnel)
{
  // 法線分布項を計算
  float D = NormalDistributionGGX(normal, halfway, roughness);

  // 幾何学的減衰項を計算
  float NdotV = max(dot(normal, cameraVector), MinCosTheta); /* 法線と視線ベクトルの内積 */
  float G = GeometricAttenuationSchlick(NdotL, NdotV, roughness);

  // 鏡面反射BRDFを計算
  float denom = 4 * NdotL * NdotV;
  return (D * G * fresnel) * (1 / denom);
}


// CalcBRDFの計算結果
struct BRDFResult
{
	vec3 diffuse;  // 拡散反射
	vec3 specular; // 鏡面反射
};
/**
* 鏡面反射BRDFと拡散反射BRDFをまとめて計算する
*
* @param normal			法線
* @param fresnel0		角度0のフレネル値
* @param cameraVector	カメラの視線方向ベクトル
* @param direction		入射光線の方向ベクトル
* @param color			入射光線の色
*
* @return 鏡面反射と拡散反射の強さ
*/
BRDFResult CalcBRDF(vec3 normal, vec3 fresnel0,
  vec3 cameraVector, vec3 direction, vec3 color)
{
  // フレネルを計算
  vec3 H = normalize(direction + cameraVector);
  vec3 F = FresnelSchlick(fresnel0, max(dot(cameraVector, H), 0));

  // GGXで鏡面反射を計算
  float NdotL = max(dot(normal, direction), MinCosTheta); /* 法線と入射光線の光源方向ベクトルの内積 */
  vec3 specularBRDF = SpecularBRDF(
	normal, H, Roughness, cameraVector, NdotL, F);
  vec3 specular = color * specularBRDF * NdotL;

  // 正規化ランバートで拡散反射を計算
  float diffuseBRDF = NdotL / PI;
  /* 角度90°ではFが1.0に、(1 - F)は0になる
	※角度90°では拡散反射は起きず、入射光は100%鏡面反射 */
  /* メタリックが1(金属マテリアル)のとき、拡散反射が無効化される
	※「金属は拡散反射を起こさない」という物理現象を再現するため */
  vec3 diffuse = color * diffuseBRDF * (1 - F) * (1 - Metallic);
 
  return BRDFResult(diffuse, specular);
}


/**
* エントリーポイント
*/
void main()
{
	vec4 c = texture(texColor, inTexcoord);

	// カットオフ値が指定されている場合、アルファがその値未満の場合は描画をキャンセル
	if (alphaCutoff > 0) {
		if (c.a < alphaCutoff) {
			discard; // フラグメントを破棄(キャンセル)
		}
		c.a = 1; // 破棄されなかったら不透明にする
    }

	// テクスチャのガンマ補正を解除
	c.rgb = pow(c.rgb, vec3(Gamma));

	outColor = c * color;
	
	// 線形補間によって長さが1ではなくなっているので、正規化して長さを1に復元する
	/* 頂点シェーダーとフラグメントシェーダー間で値が変わってしまうためもう一度正規化	
		(線形保管されて長さが１じゃなくなるから) */
	vec3 normal = ComputeWorldNormal();	

	// 光源の方向
	vec3 diffuse = vec3(0); // 拡散光の明るさの合計

	// 視線ベクトル
	vec3 cameraPos = vec3(cameraTransformMatrix[3]);
	vec3 cameraVector = normalize(cameraPos - inPosition);
	
	// 角度0のフレネル値
	/* 光線が法線と平行な角度(θ=0)で入射した場合の鏡面反射係数 */
	vec3 fresnel0 = mix(vec3(0.04), outColor.rgb, Metallic);
	
	vec3 specular = vec3(0); // 鏡面反射光の明るさの合計

	// 光源の処理を行う
	for (int i = 0; i < lightCount; ++i) {
		vec3 direction = lightList[i].position - inPosition;
	
		// 光源までの距離
		float sqrDistance = dot(direction, direction);	/* 内積によって距離の２乗を求める */
		float distance = sqrt(sqrDistance);
  	
		// 方向を正規化して長さを1にする
		direction = normalize(direction);
    
		float illuminance = 1;

		// 照射角度が0より大きければスポットライトとみなす
		if (lightList[i].coneAngle > 0) {
			// ライトからフラグメントへ向かうベクトル と スポットライトのベクトル のなす角を計算
			// 角度がconeAngle以上なら範囲外
			float angle = acos(dot(-direction, lightList[i].direction));
	
			if (angle >= lightList[i].coneAngle) {
				continue; // 照射範囲外
			}
	
			// 最大照射角度のとき0, 減衰開始角度のとき1になるように補間
		    float a = 
				min((lightList[i].coneAngle - angle) / (lightList[i].coneAngle - lightList[i].falloffAngle), 1);
		    illuminance *= a;
		} // if coneAngle

		// ライトの最大距離を制限
		const float radius = lightList[i].radius;
		const float smoothFactor = clamp(1 - pow(distance / radius, 4), 0, 1);
		illuminance *= smoothFactor * smoothFactor;	
		
		// 逆2乗の法則によって明るさを減衰させる
		/* 0除算が起きないように1を足している
			(距離が０の時に指定した明るさの最大値が出ることが保障されている) */
		illuminance /= sqrDistance + 1;
		
		// 拡散反射と鏡面反射を計算
		vec3 lightColor = lightList[i].color * illuminance;
	    BRDFResult result = CalcBRDF(normal, fresnel0, cameraVector, direction, lightColor);
	    diffuse += result.diffuse;
	    specular += result.specular;
	} // for lightCount

	// 影を計算
	float shadow = texture(texShadow, inShadowTexcoord).r;

	// 平行光源の拡散反射と鏡面反射を計算
	/*「フラグメントから見た光の向き」が必要なので、向きを逆にする*/
	BRDFResult result = CalcBRDF(normal, fresnel0, cameraVector,
	  -directionalLight.direction, directionalLight.color);
	specular += result.specular * shadow;
	diffuse += result.diffuse * shadow;

	// アンビエントライトの明るさを計算
	/* アンビエントライトは「すべての方向からの光の平均値」なので、
		フレネル値も「物体を見るときの平均的な角度」を選ぶ必要がある */
	/* 球体を正面から見たとき、角度45°境界の内側と外側の見かけの面積は等しくなる */
	vec3 f45 = fresnel0 + (1 - fresnel0) * 0.0021555; // 角度45°のフレネル値
	specular += ambientLight * f45;
	diffuse += ambientLight * (1 - f45) * (1 - Metallic);

    // 拡散光の影響を反映
	outColor.rgb *= diffuse;

	// 鏡面反射の影響を反映
	outColor.rgb += specular;

	// 発光色を反映
	outColor.rgb += 
		(useTexEmission == 1 ? // エミッションテクスチャ有無
			texture(texEmission, inTexcoord).rgb * emission.rgb : emission.rgb);

	// ガンマ補正を行う
	outColor.rgb = pow(outColor.rgb, vec3(1 / Gamma));

	//カメラのZ座標を正規化して、[-1, 1]の範囲に変換
	float zNormalized = (gl_FragCoord.z * 2.0) - 1.0;
	//正規化されたZ座標を元の遠近クリップ面の範囲に戻す
	gl_FragDepth = (gl_DepthRange.diff * zNormalized + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}