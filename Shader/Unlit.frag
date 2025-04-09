/**
* @file Unlit.frag
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition;	// ワールド座標
layout(location=1) in vec2 inTexcoord;	// テクスチャ座標

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;	// 基本色テクスチャ
layout(binding=1) uniform sampler2D texEmission;// 発光色テクスチャ

// 出力する色データ
out vec4 outColor;

// プログラムからの入力
layout(location=99)  uniform float alphaCutoff; // フラグメントを破棄する境界値
layout(location=100) uniform vec4 color;		// 物体の色
layout(location=101) uniform int useTexEmission;// エミッションテクスチャの有無
layout(location=102) uniform vec4 emission;		// 物体の発光色


const float Gamma = 2.2;		// ガンマ値(2.2はCRTモニターのガンマ値)


/**
* エントリーポイント
*/
void main()
{
	vec4 c = texture(texColor, inTexcoord);

	// カットオフ値が指定されている場合、アルファがその値未満の場合は描画をキャンセル
	if (alphaCutoff > 0) {
		if (c.a < alphaCutoff) {
			discard; // フラグメントを破棄(キャンセル)
		}
		c.a = 1; // 破棄されなかったら不透明にする
    }

	// テクスチャのガンマ補正を解除
	c.rgb = pow(c.rgb, vec3(Gamma));

	outColor = c * color;
	
	// 発光色を反映
	outColor.rgb += 
		(useTexEmission == 1 ? // エミッションテクスチャ有無
			texture(texEmission, inTexcoord).rgb * emission.rgb : emission.rgb);

	// ガンマ補正を行う
	outColor.rgb = pow(outColor.rgb, vec3(1 / Gamma));

	//カメラのZ座標を正規化して、[-1, 1]の範囲に変換
	float zNormalized = (gl_FragCoord.z * 2.0) - 1.0;
	//正規化されたZ座標を元の遠近クリップ面の範囲に戻す
	gl_FragDepth = (gl_DepthRange.diff * zNormalized + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}