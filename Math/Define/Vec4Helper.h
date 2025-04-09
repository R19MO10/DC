/**
* Vec4Helper.h
*/
#ifndef VEC4HELPER_H_INCLUDED
#define VEC4HELPER_H_INCLUDED
#include "Vec4Math.h"
#include <cmath>

// ���`���
inline vec4 lerp(const vec4& start, const vec4& end, float alpha) {
    return (1.0f - alpha) * start + alpha * end;
}

// ���ρi�l���Q�悷��j
inline float dot(const vec4& a, const vec4& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// ����
inline float length(const vec4& v) {
    return sqrt(dot(v, v));
}

// ����
inline float distance(const vec4& a, const vec4& b) {
    const vec4 result = b - a;
    return length(result);
}

// ���K��
inline vec4 normalize(const vec4& v) {
    const float result = 1.0f / length(v);
    return v * result;
}

#endif // VEC4HELPER_H_INCLUDED