/**
* @file SaoCalc.frag
*/
#version 450

// シェーダへの入力
layout(location=1) in vec2 inTexcoord;  // テクスチャ座標

// 出力する色データ
out float outColor;

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texLinearDepth; // 深度テクスチャ

// プログラムからの入力
layout(location=200) uniform vec4 saoInfo;
#define SAO_Radius2             (saoInfo.x) // サンプリング半径(m)の2乗
#define SAO_Radius_in_Textures  (saoInfo.y) // サンプリング半径(テクスチャ座標)
#define SAO_Bias                (saoInfo.z) // 平面とみなすコサイン値
#define SAO_Intensity           (saoInfo.w) // AOの強度

// NDC座標をビュー座標に変換するためのパラメータ
layout(location=201) uniform vec2 ndcToView;


const float TwoPI = 3.14159265 * 2; // 2π(360度)

// SAOの論文によると、サンプル数未満の素数を指定すると、SAOの品質が向上する
const float SamplingCount = 11;     // サンプル数（1ピクセルのAO計算で取得するサンプル数）
const float SpiralTurns = 7;        // サンプル点の回転回数（サンプリング位置を決める螺旋の巻き回数）

const int MaxMipLevel = 3;          // texDepthに割り当てるテクスチャの最大ミップレベル

// ミップレベルが切り替わり始めるサンプル点の距離(2^logMipOffsetピクセル)
const int LogMipOffset = 3;


/**
* ビュー座標系の座標を計算
*
* @param uv             テクスチャ座標
* @param linearDepth    深度値
*
* @return   ピクセルの視点座標
*/
vec3 GetViewSpacePosition(vec2 uv, float linearDepth)
{
    // テクスチャ座標からNDC座標に変換
    vec2 ndc = uv * 2 - 1;

    // NDC座標から視点からの距離1mの場合の視点座標に変換
    /* 視点から線形距離1mの位置にスクリーンを表示した場合のuv座標 */
    vec2 viewOneMeter = ndc * ndcToView;
    
    // 距離が深度値の場合の視点座標に変換
    return vec3(viewOneMeter * linearDepth, -linearDepth);
}


/**
* SAO(Scalable Ambient Obscurance)により遮蔽率を求める
*/
void main()
{
    // ピクセルの視点座標と法線を求める
    float depth = textureLod(texLinearDepth, inTexcoord, 0).x;
    vec3 positionVS = GetViewSpacePosition(inTexcoord, depth);
    /* X軸方向とY軸方向の視点座標の差分を外積することで、平面の法線ベクトルを計算する */
    vec3 normalVS = normalize(cross(dFdx(positionVS), dFdy(positionVS)));

    // 疑似的なランダム値を生成を行い開始角度に指定
    /* フラグメントごとに回転の開始角度をずらすことで見た目を改善する */
    const ivec2 iuv = ivec2(gl_FragCoord.xy);
    const float startAngle = mod((3 * iuv.x ^ iuv.y + iuv.x * iuv.y) * 10, TwoPI);
    
    // ワールド座標系とスクリーン座標系のサンプリング半径
    float radiusTS = 
        SAO_Radius_in_Textures / depth;   // テクスチャ座標系のサンプリング半径
    float pixelsSS = 
        radiusTS * textureSize(texLinearDepth, 0).y; // スクリーン座標系のサンプリング半径
    
    float occlusion = 0; // 遮蔽率
    for (int i = 0; i < SamplingCount; ++i) {
        // サンプル点の角度と距離を求める
        float ratio = (float(i) + 0.5) * (1.0 / SamplingCount);
        float angle = ratio * (SpiralTurns * TwoPI) + startAngle;
        vec2 unitOffset = vec2(cos(angle), sin(angle)); 
        
        // サンプル点の視点座標を求める
        vec2 uv = inTexcoord + ratio * radiusTS * unitOffset;
        
        // 距離が遠いほど高いミップレベルを選択
        /* 最も重要度の高いビットからミップレベルを選択する 
           （例：1のMSBは0、100のMSBは7、10000のMSBは13）
           （整数値が15以下の場合、MSBがlogMipOffsetの値3以下になるので、
           　距離15ピクセル以下ならミップレベル0の画像が使われる）*/
        /* MSBを「おおまかな距離を示すパラメータ」として使っている。
        　 実際に、MSBが1増えるごとにピクセル距離はおよそ2倍になる */
        int mipLevel = clamp(findMSB(int(ratio * pixelsSS)) - LogMipOffset, 0, MaxMipLevel);
        
        // サンプル点の視点座標を求める
        float sampleDepth = textureLod(texLinearDepth, uv, mipLevel).x;
        vec3 samplePositionVS = GetViewSpacePosition(uv, sampleDepth);

        // サンプル点へのベクトルと法線のコサインを求める
        /* コサインが1(垂直)に近いほど遮蔽率が上がる */
        vec3 v = samplePositionVS - positionVS;
        float vn = dot(v, normalVS); // 角度による遮蔽率
        
        // サンプル点が中心に近いほど遮蔽率が上がる
        float vv = dot(v, v);
        float f = max(SAO_Radius2 - vv, 0); // 距離による遮蔽率
        
        // サンプル点までの距離とコサインからAOを求める
        occlusion += f * f * f * max((vn - SAO_Bias) / (vv + 0.001), 0);
    }
    
    // 平均値を求め、AOの強さを乗算する
    occlusion = min(1.0, occlusion / SamplingCount * SAO_Intensity);

    // 水平または垂直に近い面のオクルージョンを平均化する。
    /* 隣接するピクセルとの深度値の差が少ない場合、
       隣接ピクセルとの間で遮蔽率を平均化する */
    /* スクリーン座標が偶数のときは差分の半分を加算、
       奇数のときは差分の半分を減算する */
    if (abs(dFdx(positionVS.z)) < 0.02) {
        occlusion -= dFdx(occlusion) * ((iuv.x & 1) - 0.5);
    } 
    if (abs(dFdy(positionVS.z)) < 0.02) {
        occlusion -= dFdy(occlusion) * ((iuv.y & 1) - 0.5);
    }

    outColor = occlusion;
}
