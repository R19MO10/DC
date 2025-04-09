/**
* @file Simple2D.frag
*/
#version 450

// シェーダへの入力
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
  outColor = texture(texColor, inTexcoord);
  outColor.a = 1;
}