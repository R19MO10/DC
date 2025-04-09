/**
* @ AngleMath.h
*/
#ifndef ANGLEMATH_H_INCLUDED
#define ANGLEMATH_H_INCLUDED

#include "Vec3Math.h"

constexpr float PI = 3.1415926535f;	// �~����

constexpr float halfPI = PI / 2;	// 0.5��
constexpr float twoPI = PI * 2;		// 2��
constexpr float invPI = 1 / PI;		// �~�����̋t��


// �x���@���ʓx�@�ɕϊ�
inline constexpr float degrees_to_radians(float degree) {
	return degree * PI / 180.0f;
}
inline constexpr vec3 degrees_to_radians(vec3 degree) {
	return vec3(
		degree.x * PI / 180.0f,
		degree.y * PI / 180.0f,
		degree.z * PI / 180.0f);
}

// �ʓx�@��x���@�ɕϊ�
inline constexpr float radians_to_degrees(float radian) {
	return radian * 180.0f / PI;
}
inline constexpr vec3 radians_to_degrees(vec3 radian) {
	return vec3(
		radian.x * 180.0f / PI,
		radian.y * 180.0f / PI,
		radian.z * 180.0f / PI);
}


// �x���@�p���K��
// �O�`�R�U�O�͈̔͂Ƀf�O���[�p�����߂�
inline constexpr float normalize_degrees(float degree) {
	while (degree >= 360.0f) { degree -= 360.0f; }
	while (degree < 0) { degree += 360.0f; }
	return degree;
}
inline constexpr vec3 normalize_degrees(vec3 degree) {
	for (int i = 0; i < 3; ++i) {
		while (degree[i] >= 360.0f) { degree[i] -= 360.0f; }
		while (degree[i] < 0) { degree[i] += 360.0f; }
	}
	return degree;
}

// �ʓx�@�p���K��
// �O�`�Q�΂͈̔͂̃��W�A���p�Ɏ��߂�
inline constexpr float normalize_radian(float radian) {
	while (radian >= PI * 2) { radian -= PI * 2; }
	while (radian < 0) { radian += PI * 2; }
	return radian;
}
inline constexpr vec3 normalize_radian(vec3 radian) {
	for (int i = 0; i < 3; ++i) {
		while (radian[i] >= PI * 2) { radian[i] -= PI * 2; }
		while (radian[i] < 0) { radian[i] += PI * 2; }
	}
	return radian;
}

#endif // ANFLEMATH_H_INCLUDED