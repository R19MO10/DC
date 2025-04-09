/**
* @file ParticleEmission.frag
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
	outColor = texture(texColor, inTexcoord);

	// 発光色を反映
	if (inColor.a > 1) {	// エミッション強度の有無
		outColor.rgb *= inColor.rgb;
		outColor.rgb *= (inColor.a - 1) * 0.1f;
	}
	else {
		outColor.rgb = vec3(0);
	}
}