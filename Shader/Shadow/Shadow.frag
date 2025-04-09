/**
* @file Shadow.frag
*/
#version 450

// シェーダへの入力
layout(location=1) in vec2 inTexcoord; // テクスチャ座標

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;

// プログラムからの入力
layout(location=99) uniform float alphaCutoff; // フラグメントを破棄する境界値


/**
* エントリーポイント
*/
void main()
{
	float alpha = texture(texColor, inTexcoord).a;
	
	// カットオフ値が指定されている場合、アルファがその値未満の場合は描画をキャンセル
	if (alphaCutoff > 0) {
		if (alpha < alphaCutoff) {
			discard; // フラグメントを破棄
		}
	}
}