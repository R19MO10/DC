/**
* @file ShadowAnimation.vert
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition;  // 頂点座標
layout(location=1) in vec2 inTexcoord;  // テクスチャ座標
layout(location=4) in vec4 inJoints;	// 使用する姿勢行列の番号
layout(location=5) in vec4 inWeights;	// 姿勢行列の重要度

// シェーダからの出力
layout(location=1) out vec2 outTexcoord; // テクスチャ座標

// プログラムからの入力
layout(location=3) uniform mat4 viewProjectionMatrix; // ビュープロジェクション行列

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
	// テクスチャ座標を設定
	outTexcoord = inTexcoord;
	
	// 姿勢行列にウェイトを掛けて加算合成し、モデル行列を作成
	mat4 matModel =
		matBones[int(inJoints.x)] * inWeights.x +
		matBones[int(inJoints.y)] * inWeights.y +
		matBones[int(inJoints.z)] * inWeights.z +
		matBones[int(inJoints.w)] * inWeights.w;

	// ウェイトが正規化されていない場合の対策([3][3]が1.0になるとは限らない)
	matModel[3][3] = dot(inWeights, vec4(1));

	// ローカル座標系からワールド座標系に変換
	gl_Position = matModel * vec4(inPosition, 1);
	gl_Position = viewProjectionMatrix * gl_Position;
}