/**
* @file DefaultLit_with_StoneShadow.frag
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

// 影を作成する落下石の最大数
#define MAX_FALLSTONE 16

// 落下石の数
layout(location=120) uniform int fallStoneCount;

// xy: XZ平面上の座標
// z:  石の半径
// w:  影の濃度
layout(location=121) uniform vec4 fallStoneInfo[MAX_FALLSTONE];


const float PI = 3.14159265;	// π
const float Gamma = 2.2;		// ガンマ値(2.2はCRTモニターのガンマ値)
const float MaxFallStoneShadowIntensity = 0.8;	// 落下石の影の濃度の最大値
const float FallStoneShadowFadeStartRatio = 0.8;// 落下石の影のフェードを始める中心からの割合


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

	// 光源の処理を行う
	for (int i = 0; i < lightCount; ++i) {
		vec3 direction = lightList[i].position - inPosition;
	
		// 光源までの距離
		float sqrDistance = dot(direction, direction);	/* 内積によって距離の２乗を求める */
		float distance = sqrt(sqrDistance);
  	
		// 方向を正規化して長さを1にする
		direction = normalize(direction);
    
		// ランベルトの余弦則を使って明るさを計算
		/* a,bの長さを１にしておけばcosθ = a・bにする事ができる */
		/* 明るさがマイナスにならないように最小値を0にしている */
		float theta = max(dot(direction, normal), 0);
  	
		// ランバート反射による反射光のエネルギー量を入射光と等しくするためにπで割る
		float illuminance = theta / PI;
  	
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
		float radius = lightList[i].radius;
		float smoothFactor = clamp(1 - pow(distance / radius, 4), 0, 1);
		illuminance *= smoothFactor * smoothFactor;	
	
		// 逆2乗の法則によって明るさを減衰させる
		/* 0除算が起きないように1を足している
			(距離が０の時に指定した明るさの最大値が出ることが保障されている) */
		illuminance /= sqrDistance + 1;
	
		// 拡散光の明るさを加算
		diffuse += lightList[i].color * illuminance;
	} // for lightCount

	// 影を計算
	float shadow = texture(texShadow, inShadowTexcoord).r;

	// 平行光源の明るさを計算
	/*「フラグメントから見た光の向き」が必要なので、向きを逆にする */
	float theta = max(dot(-directionalLight.direction, normal), 0);
	float illuminance = theta / PI;
	diffuse += directionalLight.color * illuminance * shadow;

	// アンビエントライトの明るさを計算
	diffuse += ambientLight;

    // 拡散光の影響を反映
	outColor.rgb *= diffuse;

	// 落下石の影の処理を行う
	float fallStoneShadow = 0;	// 落下石の影
	for (int i = 0; i < fallStoneCount; ++i) {
		 // それぞれのXZ座標
		vec2 stonePosXZ = fallStoneInfo[i].xy;

		// フラグメントと石の距離を計算
		float distance = distance(inPosition.xz, stonePosXZ);

		float radius = fallStoneInfo[i].z;	// 石の半径
		float shadowAlpha = fallStoneInfo[i].w;	// 影の濃度

		// 影のフェード割合を計算
		float ratio = smoothstep(radius * FallStoneShadowFadeStartRatio, radius, distance);
		
		// フェード割合から MaxFallStoneShadowIntensity と 0 の間で線形補間し加算
		fallStoneShadow += mix(MaxFallStoneShadowIntensity, 0, ratio) * shadowAlpha;

		// 影の濃度を制限
  		fallStoneShadow = min(fallStoneShadow, MaxFallStoneShadowIntensity);
	} // for fallStoneCount
	
	// 落下石の影を反映
	outColor.rgb *= 1 - fallStoneShadow;

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