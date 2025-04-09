/**
* @file Vec4Math.h
*/
#ifndef VEC4MATH_H_INCLUDED
#define VEC4MATH_H_INCLUDED

//��s�錾
struct vec2;
struct vec3;
struct mat4;

struct vec4 {
	float x, y, z, w;

	constexpr vec4() : x(0), y(0), z(0), w(0) {}
	constexpr explicit vec4(float s) : x(s), y(s), z(s), w(s) {}
	constexpr vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	vec4(const vec2& xy, float z, float w);
	vec4(const vec3& xyz, float w);

	// �R�s�[�R���X�g���N�^
	constexpr vec4(const vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

	// vec4�̒P��-���Z�q
	constexpr vec4 operator-() { return vec4(-x, -y, -z, -w); }
	constexpr const vec4 operator-() const { return vec4(-x, -y, -z, -w); }

	// �Y�����Z�q
	constexpr float& operator[](size_t n) { return *(&x + n); }
	constexpr const float& operator[](size_t n) const { return *(&x + n); }

	// ��r���Z�q
	constexpr bool operator==(const vec4& v) const {
		return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w);
	}
	constexpr bool operator!=(const vec4& v) const {
		return !(*this == v);
	}
	constexpr bool operator==(const float& v) const {
		return (x == v) && (y == v) && (z == v) && (w == v);
	}
	constexpr bool operator!=(const float& v) const {
		return !(*this == v);
	}

	// vec4��vec2
	vec4& operator=  (const vec2& v);
	vec4& operator+= (const vec2& v);
	vec4& operator-= (const vec2& v);
	vec4& operator*= (const vec2& v);
	vec4& operator/= (const vec2& v);


	// vec4��vec3
	vec4& operator=  (const vec3& v);
	vec4& operator+= (const vec3& v);
	vec4& operator-= (const vec3& v);
	vec4& operator*= (const vec3& v);
	vec4& operator/= (const vec3& v);


	// vec4���m
	constexpr vec4& operator= (const vec4& v) {
		if (this == &v) return *this; // ���ȑ���`�F�b�N
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}
	constexpr vec4& operator+=(const vec4& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}
	constexpr vec4& operator-=(const vec4& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}
	constexpr vec4& operator*=(const vec4& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}
	constexpr vec4& operator/=(const vec4& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}


	// vec4��float
	constexpr vec4& operator+=(const float& v) { return *this += vec4(v); }
	constexpr vec4& operator-=(const float& v) { return *this -= vec4(v); }
	constexpr vec4& operator*=(const float& v) { return *this *= vec4(v); }
	constexpr vec4& operator/=(const float& v) { return *this /= vec4(v); }
};


// vec4���m
constexpr vec4 operator+(const vec4& a, const vec4& b) { return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
constexpr vec4 operator-(const vec4& a, const vec4& b) { return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
constexpr vec4 operator*(const vec4& a, const vec4& b) { return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
constexpr vec4 operator/(const vec4& a, const vec4& b) { return vec4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }


// vec4��float
constexpr vec4 operator+(const vec4& a, float b) { return vec4(a.x + b, a.y + b, a.z + b, a.w + b); }
constexpr vec4 operator+(float a, const vec4& b) { return b + a; }
constexpr vec4 operator-(const vec4& a, float b) { return vec4(a.x - b, a.y - b, a.z - b, a.w - b); }
constexpr vec4 operator-(float a, const vec4& b) { return vec4(a) - b; }
constexpr vec4 operator*(const vec4& a, float b) { return vec4(a.x * b, a.y * b, a.z * b, a.w * b); }
constexpr vec4 operator*(float a, const vec4& b) { return vec4(a) * b; }
constexpr vec4 operator/(const vec4& a, float b) { return vec4(a.x / b, a.y / b, a.z / b, a.w / b); }


// mat4��vec4�̏�Z
vec4 operator*(const mat4& m, const vec4& v);

#endif // VEC4MATH_H_INCLUDED