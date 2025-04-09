/**
* @ Mat4Math.h
*/
#ifndef MAT4MATH_H_INCLUDED
#define MAT4MATH_H_INCLUDED
#include "Vec4Math.h"

// 先行宣言
struct mat3;
struct Quaternion;

/**
* 4x4行列
*/
struct mat4
{
	vec4 data[4];

	constexpr mat4() {
		data[0] = vec4();
		data[1] = vec4();
		data[2] = vec4();
		data[3] = vec4();
	}
	constexpr mat4(const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3) {
		data[0] = v0;
		data[1] = v1;
		data[2] = v2;
		data[3] = v3;
	}
	constexpr explicit mat4(float s) {
		data[0] = vec4(s, 0, 0, 0);
		data[1] = vec4(0, s, 0, 0);
		data[2] = vec4(0, 0, s, 0);
		data[3] = vec4(0, 0, 0, s);
	}
	mat4(const mat3& m);


	// コピーコンストラクタ
	constexpr mat4(const mat4& m) {
		data[0] = m.data[0];
		data[1] = m.data[1];
		data[2] = m.data[2];
		data[3] = m.data[3];
	}

	// Quaternionからmat4への変換コンストラクタ
	explicit mat4(const Quaternion& q);

	// mat4の単項-演算子
	constexpr mat4 operator-() { return mat4(-data[0], -data[1], -data[2], -data[3]); }
	constexpr const mat4 operator-() const { return mat4(-data[0], -data[1], -data[2], -data[3]); }

	// 添字演算子
	constexpr vec4& operator[](size_t n) { return data[n]; }
	constexpr const vec4& operator[](size_t n) const { return data[n]; }

	// mat4とmat3
	mat4& operator=  (const mat3& m);
	mat4& operator+= (const mat3& m);
	mat4& operator-= (const mat3& m);
	mat4& operator*= (const mat3& m);

	// mat4同士
	constexpr mat4& operator=(const mat4& m) {
		if (this == &m) return *this; // 自己代入チェック
		data[0] = m.data[0];
		data[1] = m.data[1];
		data[2] = m.data[2];
		data[3] = m.data[3];
		return *this;
	}
	constexpr mat4& operator+=(const mat4& m) {
		data[0] += m.data[0];
		data[1] += m.data[1];
		data[2] += m.data[2];
		data[3] += m.data[3];
		return *this;
	}
	constexpr mat4& operator-=(const mat4& m) {
		data[0] -= m.data[0];
		data[1] -= m.data[1];
		data[2] -= m.data[2];
		data[3] -= m.data[3];
		return *this;
	}
	constexpr mat4& operator*=(const mat4& m) {
		const mat4 c = *this;
		data[0] = c * m[0];
		data[1] = c * m[1];
		data[2] = c * m[2];
		data[3] = c * m[3];
		return *this;
	}

	// mat4とfloat
	constexpr mat4& operator+=(const float v) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				data[i][j] += v;
			}
		}
		return *this;
	}
	constexpr mat4& operator-=(const float v) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				data[i][j] -= v;
			}
		}
		return *this;
	}
	constexpr mat4& operator*=(const float v) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				data[i][j] *= v;
			}
		}
		return *this;
	}
	constexpr mat4& operator/=(const float v) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				data[i][j] /= v;
			}
		}
		return *this;
	}
};


// mat4同士
constexpr mat4 operator+(const mat4& m0, const mat4& m1) { return mat4(m0) += m1; }
constexpr mat4 operator-(const mat4& m0, const mat4& m1) { return mat4(m0) -= m1; }
constexpr mat4 operator*(const mat4& m0, const mat4& m1) { return mat4(m0) *= m1; }

// mat4とfloat
constexpr mat4 operator+(const mat4& a, float b) {
	mat4 result = a;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] += b;
		}
	}
	return result;
}

constexpr mat4 operator+(float a, const mat4& b) {
	return b + a;
}

constexpr mat4 operator-(const mat4& a, float b) {
	mat4 result = a;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] -= b;
		}
	}
	return result;
}

constexpr mat4 operator-(float a, const mat4& b) {
	mat4 result = b;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] = a - result[i][j];
		}
	}
	return result;
}

constexpr mat4 operator*(const mat4& a, float b) {
	mat4 result = a;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] *= b;
		}
	}
	return result;
}

constexpr mat4 operator*(float a, const mat4& b) {
	return b * a;
}

constexpr mat4 operator/(const mat4& a, float b) {
	mat4 result = a;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] /= b;
		}
	}
	return result;
}

#endif	// MAT4MATH_H_INCLUDED