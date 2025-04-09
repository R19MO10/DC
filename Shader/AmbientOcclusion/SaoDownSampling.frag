/**
* @file SaoDownSampling.frag
*/
#version 450

// 出力する色データ
out float outColor;

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texLinearDepth;	// 線形深度値

// プログラムからの入力
layout(location=200) uniform int mipLevel; // コピー元のミップレベル


/**
* 線形距離を縮小
*/
void main()
{
	// 2x2テクセルのうち、最も視点に近い値を選択
	ivec2 uv = ivec2(gl_FragCoord.xy) * 2;

	// 周囲4ピクセルのうち、最も視点に近い値を選択
	float d1 = texelFetch(texLinearDepth, uv + ivec2(0, 0), mipLevel).x;
	float d2 = texelFetch(texLinearDepth, uv + ivec2(0, 1), mipLevel).x;
	float d3 = texelFetch(texLinearDepth, uv + ivec2(1, 1), mipLevel).x;
	float d4 = texelFetch(texLinearDepth, uv + ivec2(1, 0), mipLevel).x;
	outColor = min(min(d1, d2), min(d3, d4));
}