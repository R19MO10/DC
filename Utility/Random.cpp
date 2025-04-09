/**
* @file Random.cpp
*/
#include "Random.h"

#include <random>

namespace Random {

	// 乱数生成オブジェクト
	std::random_device rd;
	std::mt19937 randomEngine(rd());

	void Initialize(int seed) {
		randomEngine.seed(seed);
	}


	template <typename T>
	T Range(T min, T max) {
		if constexpr (std::is_integral<T>::value) {
			// 整数型の場合
			return std::uniform_int_distribution<T>(min, max)(randomEngine);
		}
		else if constexpr (std::is_floating_point<T>::value) {
			// 浮動小数点型の場合
			return std::uniform_real_distribution<T>(min, max)(randomEngine);
		}

		// 対応していない型の場合
		throw std::invalid_argument("Unsupported type");
	}


	template<>
	vec2Int Range<vec2Int>(vec2Int min, vec2Int max) {
		return vec2Int(
			Range(min.x, max.x),
			Range(min.y, max.y)
		);
	}
	template <>
	vec2 Range<vec2>(vec2 min, vec2 max) {
		return vec2(
			Range(min.x, max.x),
			Range(min.y, max.y)
		);
	}
	template <>
	vec3 Range<vec3>(vec3 min, vec3 max) {
		return vec3(
			Range(min.x, max.x),
			Range(min.y, max.y),
			Range(min.z, max.z)
		);
	}
	template <>
	vec4 Range<vec4>(vec4 min, vec4 max) {
		return vec4(
			Range(min.x, max.x),
			Range(min.y, max.y),
			Range(min.z, max.z),
			Range(min.w, max.w)
		);
	}

} // namespace Random