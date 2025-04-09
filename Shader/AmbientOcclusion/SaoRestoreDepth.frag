/**
* @file SaoRestoreDepth.frag
*/
#version 450

// 出力する色データ
out float outColor;

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texDepth;	// 深度値テクスチャ

// プログラムからの入力
layout(location=200) uniform vec2 cameraNearFar;
#define CameraNear	(cameraNearFar.x) // nearプレーンまでの距離
#define CameraFar	(cameraNearFar.y) // farプレーンまでの距離


/**
* 深度バッファの値からビュー空間のZ値を復元（線形距離への変換）
*/
void main()
{
	// 2x2テクセルのうち、最も視点に近い値を選択
	/* サイズを縦横1/2(面積では1/4)に縮小して計算しているのは
		テクスチャのサイズを減らし、以後の処理でテクスチャの読み取りにかかる時間を短縮するため */
	ivec2 uv = ivec2(gl_FragCoord.xy) * 2;
	// 周囲4ピクセルのうち、最も視点に近い値を選択
	/* 深度値は非線形のため単純にピクセルの平均が使えないため */
	float d1 = texelFetch(texDepth, uv + ivec2(0, 0), 0).x;
	float d2 = texelFetch(texDepth, uv + ivec2(0, 1), 0).x;
	float d3 = texelFetch(texDepth, uv + ivec2(1, 1), 0).x;
	float d4 = texelFetch(texDepth, uv + ivec2(1, 0), 0).x;
	float depth = min(min(d1, d2), min(d3, d4));
	
	// 深度値を線形に戻す
	/* 深度値の計算結果が-1～+1になるようなパラメータA, B */
	const float A = -2 * CameraFar * CameraNear / (CameraFar - CameraNear);
	const float B = (CameraFar + CameraNear) / (CameraFar - CameraNear);
	depth = 2 * depth - 1;
	outColor = A / (depth - B);
}