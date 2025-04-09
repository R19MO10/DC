/**
* @file HighPassFilter.frag
*/
#version 450

// シェーダへの入力
layout(location=1) in vec2 inTexcoord;	// テクスチャ座標

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;

// 出力する色データ
out vec4 outColor;

// プログラムからの入力
layout(location=150) uniform vec2 bloomInfo;
#define Bloom_Threshold     (bloomInfo.x) // 高輝度とみなす明るさ(しきい値)
#define Bloom_Strength      (bloomInfo.y) // ブルームの強さ


/**
* 高輝度ピクセルの色成分を取得する
*/
vec3 GetBrightnessPart(vec3 color)
{
  // RGBのうち、もっとも明るい要素の値をピクセルの輝度とする
  float brightness = max(color.r, max(color.g, color.b));

  // 高輝度成分の比率を計算
  /* 右辺のmaxは0除算エラー回避 */
  float ratio = max(brightness - Bloom_Threshold, 0) / max(brightness, 0.00001);

  // 高輝度成分を計算
  return color * ratio;
}


/**
* 高輝度成分を抽出し、縮小する
*/
void main()
{
    // テクセルサイズを計算
    vec2 oneTexel = 1 / vec2(textureSize(texColor, 0));

    // 明るい成分を計算、4x4ピクセルの縮小ぼかし処理
    /* 中心のため、4掛けて明るくする */
    outColor.rgb = GetBrightnessPart(texture(texColor, inTexcoord).rgb) * 4;

    // 四方
    outColor.rgb += GetBrightnessPart(
        texture(texColor, inTexcoord + vec2(-oneTexel.x, oneTexel.y)).rgb); /* 左上 */
    outColor.rgb += GetBrightnessPart(
        texture(texColor, inTexcoord + vec2( oneTexel.x, oneTexel.y)).rgb); /* 左下 */
    outColor.rgb += GetBrightnessPart(
        texture(texColor, inTexcoord + vec2(-oneTexel.x,-oneTexel.y)).rgb); /* 右上 */
    outColor.rgb += GetBrightnessPart(
        texture(texColor, inTexcoord + vec2( oneTexel.x,-oneTexel.y)).rgb); /* 右下 */

    // ブルームの強さを乗算
    /* 値を平均化のために 1/8 にしている */
    outColor.rgb *= (1.0 / 8.0) * Bloom_Strength;   

    outColor.a = 1;   /* 使用しないから1 */
}