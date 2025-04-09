/**
* @file DefaultLit.vert
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition;  // 頂点座標
layout(location=1) in vec2 inTexcoord;  // テクスチャ座標
layout(location=2) in vec3 inNormal;    // 法線ベクトル
layout(location=3) in vec4 inTangent;   // タンジェントベクトル

// シェーダからの出力
layout(location=0) out vec3 outPosition;    // ワールド座標
layout(location=1) out vec2 outTexcoord;    // テクスチャ座標
layout(location=2) out vec3 outNormal;      // 法線ベクトル
layout(location=3) out vec4 outTangent;     // タンジェントベクトル
layout(location=4) out vec3 outShadowTexcoord; // シャドウテクスチャ座標

// プログラムからの入力
layout(location=0) uniform mat4 transformMatrix;   // 描画モデルの座標変換行列
layout(location=1) uniform mat4 cameraTransformMatrix;  // カメラの座標変換行列

// x: 視野角による水平拡大率
// y: 視野角による垂直拡大率
// z: 遠近法パラメータA
// w: 遠近法パラメータB
layout(location=2) uniform vec4 cameraInfo;

layout(location=10) uniform mat4 shadowTextureMatrix; // シャドウテクスチャ行列
layout(location=11) uniform float shadowNormalOffset; // 座標を法線方向にずらす量


/**
* エントリーポイント
*/
void main()
{
    outTexcoord = inTexcoord;

    // 拡大縮小と回転と平行移動
    gl_Position = transformMatrix * vec4(inPosition, 1);

    outPosition = gl_Position.xyz;

    // ワールド法線を計算（影響は回転のみ）
    mat3 matNormal = transpose(inverse(mat3(transformMatrix)));
    /* 
     1.「拡大率の逆数を掛けた回転行列」を作るには、
        inverse(インバース)してtranspose(トランスポーズ)する。(逆転置行列)

     2. 法線に逆転置行列を掛けた結果は単位ベクトルにならない(ことがある)ので、
        正規化する必要がある。 
    */
    outNormal = normalize(matNormal * inNormal);

    // ワールドタンジェントを計算
    /* 平行移動を含まないmatNormal行列を使ってワールド座標系に変換 */
    outTangent.xyz = normalize(matNormal * inTangent.xyz);  
    outTangent.w = inTangent.w;

    // シャドウテクスチャ座標を計算
    outShadowTexcoord = outPosition + outNormal * shadowNormalOffset;
    outShadowTexcoord = vec3(shadowTextureMatrix * vec4(outShadowTexcoord, 1));

    // ワールド座標系からビュー座標系に変換
    vec3 pos = outPosition - vec3(cameraTransformMatrix[3]);
    gl_Position.xyz = pos * mat3(cameraTransformMatrix);
  
    // 視野角を反映
    gl_Position.xy *= cameraInfo.xy;
  
    // 遠近法を有効にする
    gl_Position.w = -gl_Position.z;
  
    // 深度値を補正（深度値の計算結果が-1～+1になるようにする）
    gl_Position.z = -gl_Position.z * cameraInfo.w + cameraInfo.z; 
}