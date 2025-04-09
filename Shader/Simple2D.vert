/**
* @file Simple2D.vert
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition; // 頂点座標
layout(location=1) in vec2 inTexcoord; // テクスチャ座標

// シェーダからの出力
layout(location=0) out vec3 outPosition;    // ワールド座標
layout(location=1) out vec2 outTexcoord;    // テクスチャ座標

// プログラムからの入力
layout(location=0) uniform mat4 transformMatrix; // 描画モデルの座標変換行列
layout(location=1) uniform mat4 viewProjectionMatrix; // ビュープロジェクション行列


/**
* エントリーポイント
*/
void main()
{
	// テクスチャ座標を設定
	outTexcoord = inTexcoord;

	// ローカル座標系からワールド座標系に変換
	outPosition = vec3(transformMatrix * vec4(inPosition, 1));
	gl_Position = viewProjectionMatrix * vec4(outPosition, 1); 
}