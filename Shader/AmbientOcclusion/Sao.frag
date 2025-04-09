/**
* @file Sao.frag
*/
#version 450

layout(location=1) in vec2 inTexcoord;

// 出力する色データ
out vec4 outColor;

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texShield;	// SAOテクスチャ（遮蔽率）
layout(binding=1) uniform sampler2D texColor;	// SAOを適応するスクリーンテクスチャ


/**
* SAOをゲーム画面に適応する
*/
void main()
{
	// 遮蔽率を取得
	float occlusion = texture(texShield, inTexcoord).x;
	
	outColor = texture(texColor, inTexcoord);
	outColor = vec4(vec3(outColor) * (1 - occlusion), outColor.w);
}