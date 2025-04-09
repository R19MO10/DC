/**
* @file DownSampling.frag
*/
#version 450

// シェーダへの入力
layout(location=1) in vec2 inTexcoord;	// テクスチャ座標

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;

// 出力する色データ
out vec4 outColor;


/**
* 縮小シェーダ
*/
void main()
{
  // サンプリングポイントと重みの配列
  const float offsets[] = { -2.75, -0.92, 0.92, 2.75 };
  const float weights[] = { 0.1087, 0.3913, 0.3913, 0.1087 };

  // テクセルサイズを計算
  vec2 oneTexel = 1 / vec2(textureSize(texColor, 0));

  // 8x8ピクセルのぼかし処理を行う  
    //※中心からわずかに内側にずらしている
  outColor = vec4(0);
  for (int y = 0; y < 4; ++y) {
    float ty = inTexcoord.y + offsets[y] * oneTexel.y;
    for (int x = 0; x < 4; ++x) {
      float tx = inTexcoord.x + offsets[x] * oneTexel.x;
      outColor += texture(texColor, vec2(tx, ty)) * weights[x] * weights[y];
    }
  }
}