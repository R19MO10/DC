/**
* @file SaoBlur.frag
*/
#version 450

// 出力する色データ
out vec4 outColor;

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texShield;      // 遮蔽率
layout(binding=1) uniform sampler2D texLinearDepth; // 線形深度値


// ぼかし係数の配列
const float Gaussian[5] = 
  { 0.153170, 0.444893, 0.422649, 0.392902, 0.362970 };

const int BlurRadius = 4;        // ぼかし半径
const int BlurScale = 1;         // ぼかし半径の拡大率
const float EdgeDistance = 0.15; // 同じ物体とみなす深度差(単位=m)(深度差の15cmは、NvidiaのSAOの論文で使用された値)
const float EdgeSharpness = 1 / EdgeDistance; // 物体の輪郭をぼかさないための係数
const float Far = 1000;          // ファー平面の深度値


/**
* SAOを計算結果は粒状感が目立っているため、画像をぼかして粒子が目立たないようにする
*/
void main()
{
	// ピクセルの遮蔽率と深度値を取得
	ivec2 uv = ivec2(gl_FragCoord.xy);
	outColor = texelFetch(texShield, uv, 0);
	float depth = texelFetch(texLinearDepth, uv, 0).x;
	
	// 深度値がファー平面以上の場合、そのピクセルは背景なのでぼかさない
	if (depth >= Far) {
	  return;
	}

	// バイラテラル・フィルタを実行
    /* 深度値を利用した物体の輪郭判定を行い、
        輪郭(と思われる)部分を残しつつ、輪郭ではない部分をぼかす */
    outColor *= Gaussian[0] * Gaussian[0];
    float totalWeight = Gaussian[0] * Gaussian[0];

    /* サンプリング範囲は「(BlurRadius * 2 + 1)x(BlurRadius * 2 + 1)ピクセル」 */
    for (int y = -BlurRadius; y <= BlurRadius; ++y) {
        for (int x = -BlurRadius; x <= BlurRadius; ++x) {
            // 中心は取得済みなので無視する
            if (y == 0 && x == 0) {
                continue;
            }
        
            // サンプル点のウェイト(重要度)を取得
            float weight = Gaussian[abs(x)] * Gaussian[abs(y)];
        
            // 中心との深度値の差によって物体の境界を判定し、ウェイトを調整
            /* EdgeDistanceに近づくほど重要度が低下し、
                EdgeDistance以上になると重要度が0になる */
            /* 深度値の差がEdgeDistance以上ある場合は、
                サンプル点は中央ピクセルとは異なる平面に属する */
            ivec2 sampleUV = uv + ivec2(x, y) * BlurScale;
            float sampleDepth = texelFetch(texLinearDepth, sampleUV, 0).x;
            weight *= max(0.0, 1.0 - EdgeSharpness * abs(sampleDepth - depth));
        
            // 遮蔽率と合計ウェイトを更新
            outColor += texelFetch(texShield, sampleUV, 0) * weight;
            totalWeight += weight;
        }
    }
    
    outColor /= totalWeight;
}