/**
* Vec3Helper.h
*/
#ifndef VEC3HELPER_H_INCLUDED
#define VEC3HELPER_H_INCLUDED
#include "Vec3Math.h"
#include <cmath>

// 線形補間
inline vec3 lerp(const vec3& start, const vec3& end, float alpha) {
	return (1.0f - alpha) * start + alpha * end;
}

// 内積（値を２乗する）
inline float dot(const vec3& a, const vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// 長さ
inline float length(const vec3& v)
{
	return sqrt(dot(v, v));
}

// 距離
inline float distance(const vec3& a, const vec3& b)
{
	const vec3  result = b - a;
	return length(result);
}

// 正規化
inline vec3 normalize(const vec3& v)
{
	const float result = 1.0f / length(v);
	return v * result;
}

// 外積
inline vec3 cross(const vec3& a, const vec3& b)
{
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

#endif // VEC3HELPER_H_INCLUDED