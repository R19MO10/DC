/**
* Vec2Helper.h
*/
#ifndef VEC2HELPER_H_INCLUDED
#define VEC2HELPER_H_INCLUDED
#include "Vec2Math.h"
#include <cmath>

// 線形補間
inline vec2 lerp(const vec2& start, const vec2& end, float alpha) {
	return (1.0f - alpha) * start + alpha * end;
}

// 内積（値を２乗する）
inline float dot(const vec2& a, const vec2& b)
{
	return a.x * b.x + a.y * b.y;
}

// 長さ
inline float length(const vec2& v)
{
	return sqrt(dot(v, v));
}

// 距離
inline float distance(const vec2& a, const vec2& b)
{
	const vec2  result = b - a;
	return length(result);
}

// 正規化
inline vec2 normalize(const vec2& v)
{
	const float result = 1.0f / length(v);
	return v * result;
}

#endif // VEC2HELPER_H_INCLUDED