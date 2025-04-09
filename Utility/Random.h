/**
* @file Random.h
*/
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED
#include "../Math/MyMath.h"

/**
* 乱数生成機能を格納する名前空間
*/
namespace Random 
{
	/**
	* 乱数を初期化する
	*
	* @param seed 乱数の種
	*/
	void Initialize(int seed);


	/**
	* 0.0～1.0の範囲の一様乱数を取得する
	*
	* @return 生成した乱数
	*/
	template <typename T>
	inline T Value() {
		return Range<T>(T(0), T(1));
	}

	/**
	* 指定されたの範囲の一様乱数を取得する
	*
	* @param min 生成範囲の最小値
	* @param max 生成範囲の最大値
	*
	* @return 生成した乱数
	*/
	template <typename T>
	T Range(T min, T max);

	template<>
	vec2Int Range<vec2Int>(vec2Int min, vec2Int max);
	template <>
	vec2 Range<vec2>(vec2 min, vec2 max);
	template <>
	vec3 Range<vec3>(vec3 min, vec3 max);
	template <>
	vec4 Range<vec4>(vec4 min, vec4 max);

} // namespace Random

#endif // RANDOM_H_INCLUDED