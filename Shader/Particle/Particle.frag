/**
* @file Particle.frag
*/
#version 450

// シェーダへの入力
layout(location=0) in vec4 inColor;		// 基本色
layout(location=1) in vec2 inTexcoord;	// テクスチャ座標

// 出力する色データ
out vec4 outColor;

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;	// 基本色テクスチャ


/**
* エントリーポイント
*/
void main()
{
	// 基本色とテクスチャ色を混ぜ合わせる
	outColor = inColor * texture(texColor, inTexcoord);
	outColor.a = clamp(outColor.a, 0, 1);
}