/**
* Vec4Helper.h
*/
#ifndef VEC4HELPER_H_INCLUDED
#define VEC4HELPER_H_INCLUDED
#include "Vec4Math.h"
#include <cmath>

// 線形補間
inline vec4 lerp(const vec4& start, const vec4& end, float alpha) {
    return (1.0f - alpha) * start + alpha * end;
}

// 内積（値を２乗する）
inline float dot(const vec4& a, const vec4& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// 長さ
inline float length(const vec4& v) {
    return sqrt(dot(v, v));
}

// 距離
inline float distance(const vec4& a, const vec4& b) {
    const vec4 result = b - a;
    return length(result);
}

// 正規化
inline vec4 normalize(const vec4& v) {
    const float result = 1.0f / length(v);
    return v * result;
}

#endif // VEC4HELPER_H_INCLUDED