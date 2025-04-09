/**
* Vec2Helper.h
*/
#ifndef VEC2HELPER_H_INCLUDED
#define VEC2HELPER_H_INCLUDED
#include "Vec2Math.h"
#include <cmath>

// ���`���
inline vec2 lerp(const vec2& start, const vec2& end, float alpha) {
	return (1.0f - alpha) * start + alpha * end;
}

// ���ρi�l���Q�悷��j
inline float dot(const vec2& a, const vec2& b)
{
	return a.x * b.x + a.y * b.y;
}

// ����
inline float length(const vec2& v)
{
	return sqrt(dot(v, v));
}

// ����
inline float distance(const vec2& a, const vec2& b)
{
	const vec2  result = b - a;
	return length(result);
}

// ���K��
inline vec2 normalize(const vec2& v)
{
	const float result = 1.0f / length(v);
	return v * result;
}

#endif // VEC2HELPER_H_INCLUDED