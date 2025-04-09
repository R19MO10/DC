/**
* @file Emission.frag
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition; // ワールド座標
layout(location=1) in vec2 inTexcoord; // テクスチャ座標

// 出力する色データ
out vec4 outColor;

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;	// 基本色テクスチャ
layout(binding=1) uniform sampler2D texEmission;// 発光色テクスチャ

// プログラムからの入力
layout(location=99) uniform float alphaCutoff;	// フラグメントを破棄する境界値
layout(location=100) uniform vec4 color;		// 物体の色
layout(location=101) uniform int useTexEmission;// エミッションテクスチャの有無
layout(location=102) uniform vec4 emission;		// 物体の発光色


const float Gamma = 2.2;	// ガンマ値(2.2はCRTモニターのガンマ値)


/**
* エントリーポイント
*/
void main()
{
	// カラーテクスチャから透明度を取得する
	float alpha = texture(texColor, inTexcoord).a;

	// カットオフ値が指定されている場合、アルファがその値未満の場合は描画をキャンセル
	if (alphaCutoff > 0) {
		// カラーテクスチャで透明な所はフラグメントを破棄する
		if (alpha < alphaCutoff) {
			discard; // フラグメントを破棄(キャンセル)
		}
		alpha = 1; // 破棄されなかったら不透明にする
	}

	outColor.a = alpha * color.a;
	
	// 発光色を反映
	if (useTexEmission == 1) {	// エミッションテクスチャの有無
		outColor.rgb = texture(texEmission, inTexcoord).rgb * emission.rgb * emission.a;
	} else {
		outColor.rgb = emission.rgb * emission.a;
	}

	// ガンマ補正を行う
	outColor.rgb = pow(outColor.rgb, vec3(1 / Gamma));
}