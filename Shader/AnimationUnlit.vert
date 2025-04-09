/**
* @file Animation.vert
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition;  // 頂点座標
layout(location=1) in vec2 inTexcoord;  // テクスチャ座標
layout(location=4) in vec4 inJoints;	// 使用する姿勢行列の番号
layout(location=5) in vec4 inWeights;	// 姿勢行列の重要度

// シェーダからの出力
layout(location=0) out vec3 outPosition;    // ワールド座標
layout(location=1) out vec2 outTexcoord;    // テクスチャ座標

// プログラムからの入力
layout(location=1) uniform mat4 cameraTransformMatrix;  // カメラの座標変換行列

// x: 視野角による水平拡大率
// y: 視野角による垂直拡大率
// z: 遠近法パラメータA
// w: 遠近法パラメータB
layout(location=2) uniform vec4 cameraInfo;

// スケルタルアニメーション用SSBO
layout(std430, binding=1) buffer AnimationDataBlock
{
	mat4 matBones[]; // 姿勢行列の配列
};


/**
* エントリーポイント
*/
void main()
{
	outTexcoord = inTexcoord;

	// 姿勢行列にウェイトを掛けて加算合成し、モデル行列を作成
	mat4 matModel =
		matBones[int(inJoints.x)] * inWeights.x +
		matBones[int(inJoints.y)] * inWeights.y +
		matBones[int(inJoints.z)] * inWeights.z +
		matBones[int(inJoints.w)] * inWeights.w;

	// ウェイトが正規化されていない場合の対策([3][3]が1.0になるとは限らない)
	matModel[3][3] = dot(inWeights, vec4(1));

	// 拡大縮小と回転と平行移動
	gl_Position = matModel * vec4(inPosition, 1);

	outPosition = gl_Position.xyz;

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