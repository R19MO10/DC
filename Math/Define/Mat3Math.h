/**
* @ Mat3Math.h
*/
#ifndef MAT3MATH_H_INCLUDED
#define MAT3MATH_H_INCLUDED
#include "Vec3Math.h"

// 先行宣言
struct mat4;
struct Quaternion;

/**
* 3x3行列
*/
struct mat3
{
	vec3 data[3];

	// コンストラクタ
	constexpr mat3() {
		data[0] = vec3();
		data[1] = vec3();
		data[2] = vec3();
	}
	constexpr mat3(const vec3& v0, const vec3& v1, const vec3& v2) {
		data[0] = v0;
		data[1] = v1;
		data[2] = v2;
	}
	constexpr explicit mat3(float s) {
		data[0] = vec3(s, 0, 0);
		data[1] = vec3(0, s, 0);
		data[2] = vec3(0, 0, s);
	}

	// コピーコンストラクタ
	constexpr mat3(const mat3& m) {
		data[0] = m.data[0];
		data[1] = m.data[1];
		data[2] = m.data[2];
	}

	// mat4からmat3への変換コンストラクタ
	explicit mat3(const mat4& m);

	// Quaternionからmat3への変換コンストラクタ
	explicit mat3(const Quaternion& q);

	// mat3の単項-演算子
	constexpr mat3 operator-() { return mat3(-data[0], -data[1], -data[2]); }
	constexpr const mat3 operator-() const { return mat3(-data[0], -data[1], -data[2]); }

	// 添字演算子
	constexpr vec3& operator[](size_t n) { return data[n]; }
	constexpr const vec3& operator[](size_t n) const { return data[n]; }


	// mat3同士
	constexpr mat3& operator=(const mat3& m) {
		if (this == &m) return *this; // 自己代入チェック
		data[0] = m.data[0];
		data[1] = m.data[1];
		data[2] = m.data[2];
		return *this;
	}
	constexpr mat3& operator+=(const mat3& m) {
		data[0] += m.data[0];
		data[1] += m.data[1];
		data[2] += m.data[2];
		return *this;
	}
	constexpr mat3& operator-=(const mat3& m) {
		data[0] -= m.data[0];
		data[1] -= m.data[1];
		data[2] -= m.data[2];
		return *this;
	}
	constexpr mat3& operator*=(const mat3& m) {
		const mat3 c = *this;
		data[0] = c * m[0];
		data[1] = c * m[1];
		data[2] = c * m[2];
		return *this;
	}

	// mat3とfloat
	constexpr mat3& operator+=(const float v) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				data[i][j] += v;
			}
		}
		return *this;
	}
	constexpr mat3& operator-=(const float v) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				data[i][j] -= v;
			}
		}
		return *this;
	}
	constexpr mat3& operator*=(const float v) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				data[i][j] *= v;
			}
		}
		return *this;
	}
	constexpr mat3& operator/=(const float v) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				data[i][j] /= v;
			}
		}
		return *this;
	}
};


// mat3同士
inline constexpr mat3 operator+(const mat3& m0, const mat3& m1) { return mat3(m0) += m1; }
inline constexpr mat3 operator-(const mat3& m0, const mat3& m1) { return mat3(m0) -= m1; }
inline constexpr mat3 operator*(const mat3& m0, const mat3& m1) { return mat3(m0) *= m1; }

// mat3とfloat
constexpr mat3 operator+(const mat3& a, float b) {
	mat3 result = a;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result[i][j] += b;
		}
	}
	return result;
}

constexpr mat3 operator+(float a, const mat3& b) {
	return b + a;
}

constexpr mat3 operator-(const mat3& a, float b) {
	mat3 result = a;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result[i][j] -= b;
		}
	}
	return result;
}

constexpr mat3 operator-(float a, const mat3& b) {
	mat3 result = b;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result[i][j] = a - result[i][j];
		}
	}
	return result;
}

constexpr mat3 operator*(const mat3& a, float b) {
	mat3 result = a;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result[i][j] *= b;
		}
	}
	return result;
}

constexpr mat3 operator*(float a, const mat3& b) {
	return b * a;
}

constexpr mat3 operator/(const mat3& a, float b) {
	mat3 result = a;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result[i][j] /= b;
		}
	}
	return result;
}

#endif // MAT3MATH_H_INCLUDED