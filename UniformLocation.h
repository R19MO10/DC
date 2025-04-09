/**
* @file UniformLocation.h
*/
#ifndef UNIFORMLOCATION_H_INCLUDED
#define UNIFORMLOCATION_H_INCLUDED
#include "glad/glad.h"

// シェーダで使えるライトの数
constexpr size_t MaxShaderLightCount = 16;

// シェーダで使える落下石の数
constexpr size_t MaxShaderFallStoneCount = 16;

// ロケーション番号
namespace Loc {
	constexpr GLint TransformMatrix			= 0;
	constexpr GLint CameraTransformMatrix	= 1;
	constexpr GLint CameraInfo				= 2;
	constexpr GLint ViewProjectionMatrix	= 3;
	constexpr GLint ShadowTextureMatrix		= 10;
	constexpr GLint ShadowNormalOffset		= 11;

	constexpr GLint AlphaCutoff				= 99;

	constexpr GLint Color					= 100;
	constexpr GLint UseTexEmission			= 101;
	constexpr GLint Emission				= 102;
	constexpr GLint RoughnessAndMetallic	= 103;

	constexpr GLint AmbientLight				= 107;
	constexpr GLint DirectionalLightColor		= 108;
	constexpr GLint DirectionalLightDirection	= 109;

	// シェーダの落下石配列
	namespace FallStone {
		constexpr GLint FallStoneCount = 120;
		constexpr GLint FallStoneInfo = 121;
		/* 121 + MaxShaderFallStoneCount を使用済み */
	}

	// ポストプロセス
	namespace Bloom {
		constexpr GLint BloomInfo = 150;
	}
	namespace Sao {
		constexpr GLint CameraNearFar	= 200;
		constexpr GLint MipLevel		= 200;
		constexpr GLint SaoInfo			= 200;
		constexpr GLint NdcToView		= 201;
	}
}

// バインド番号
namespace Bind {
	// SSBOのバインディングポイント
	namespace SSBO {
		constexpr GLint Light		= 0;
		constexpr GLint Animation	= 1;
		constexpr GLint Particle	= 2;
	}

	// テクスチャのバインディングポイント
	namespace Texture {
		constexpr GLint Color	 = 0;
		constexpr GLint Emission = 1;
		constexpr GLint Normal	 = 2;
		constexpr GLint Shadow	 = 3;
	}
}

#endif // UNIFORMLOCATION_H_INCLUDED