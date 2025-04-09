/**
* @file UpSampling.frag
*/
#version 450

// シェーダへの入力
layout(location=1) in vec2 inTexcoord;	// テクスチャ座標

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;

// 出力する色データ
out vec4 outColor;


/**
* 拡大シェーダ
*/
void main()
{
	// テクセルサイズを計算
	vec2 halfTexel = 0.5 / vec2(textureSize(texColor, 0));

	// 3x3ピクセルのぼかし処理を行う
	/* 中央のピクセルは4倍、上下左右は2倍、ななめ方向は1倍の重みで合成 */
	outColor  = texture(texColor, inTexcoord + vec2(-halfTexel.x, halfTexel.y));
	outColor += texture(texColor, inTexcoord + vec2( halfTexel.x, halfTexel.y));
	outColor += texture(texColor, inTexcoord + vec2( halfTexel.x,-halfTexel.y));
	outColor += texture(texColor, inTexcoord + vec2(-halfTexel.x,-halfTexel.y));
	outColor *= 1.0 / 4.0; // 平均化
}