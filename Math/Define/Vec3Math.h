/**
* @file Vec3Math.h
*/
#ifndef VEC3MATH_H_INCLUDED
#define VEC3MATH_H_INCLUDED

//先行宣言
struct vec2;
struct vec4;
struct mat3;

struct vec3 {
	float x, y, z;

	constexpr vec3() : x(0), y(0), z(0) {}
	constexpr explicit vec3(float s) : x(s), y(s), z(s) {}
	constexpr vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	vec3(const vec2& xy, float z);

	// コピーコンストラクタ
	constexpr vec3(const vec3& v) : x(v.x), y(v.y), z(v.z) {}

	// vec4からvec3への変換コンストラクタ
	explicit vec3(const vec4& v);

	// 単項-演算子
	constexpr vec3 operator-() { return vec3(-x, -y, -z); }
	constexpr const vec3 operator-() const { return vec3(-x, -y, -z); }

	// 添字演算子
	constexpr float& operator[](size_t n) { return *(&x + n); }
	constexpr const float& operator[](size_t n) const { return *(&x + n); }

	// 比較演算子
	constexpr bool operator==(const vec3& v) const {
		return (x == v.x) && (y == v.y) && (y == v.z);
	}
	constexpr bool operator!=(const vec3& v) const {
		return !(*this == v);
	}
	constexpr bool operator==(const float& v) const {
		return (x == v) && (y == v) && (z == v);
	}
	constexpr bool operator!=(const float& v) const {
		return !(*this == v);
	}

	// vec3とvec2
	vec3& operator=  (const vec2& v);
	vec3& operator+= (const vec2& v);
	vec3& operator-= (const vec2& v);
	vec3& operator*= (const vec2& v);
	vec3& operator/= (const vec2& v);


	// vec3同士
	constexpr vec3& operator= (const vec3& v) {
		if (this == &v) return *this; // 自己代入チェック
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}
	constexpr vec3& operator+=(const vec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
	constexpr vec3& operator-=(const vec3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
	constexpr vec3& operator*=(const vec3& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}
	constexpr vec3& operator/=(const vec3& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}


	// vec3とfloat
	constexpr vec3& operator+=(const float& v) { return *this += vec3(v); }
	constexpr vec3& operator-=(const float& v) { return *this -= vec3(v); }
	constexpr vec3& operator*=(const float& v) { return *this *= vec3(v); }
	constexpr vec3& operator/=(const float& v) { return *this /= vec3(v); }
};


// vec3同士
constexpr vec3 operator+(const vec3& a, const vec3& b) { return vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
constexpr vec3 operator-(const vec3& a, const vec3& b) { return vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
constexpr vec3 operator*(const vec3& a, const vec3& b) { return vec3(a.x * b.x, a.y * b.y, a.z * b.z); }
constexpr vec3 operator/(const vec3& a, const vec3& b) { return vec3(a.x / b.x, a.y / b.y, a.z / b.z); }


// vec3とfloat
constexpr vec3 operator+(const vec3& a, float b) { return vec3(a.x + b, a.y + b, a.z + b); }
constexpr vec3 operator+(float a, const vec3& b) { return b + a; }
constexpr vec3 operator-(const vec3& a, float b) { return vec3(a.x - b, a.y - b, a.z - b); }
constexpr vec3 operator-(float a, const vec3& b) { return vec3(a) - b; }
constexpr vec3 operator*(const vec3& a, float b) { return vec3(a.x * b, a.y * b, a.z * b); }
constexpr vec3 operator*(float a, const vec3& b) { return b * a; }
constexpr vec3 operator/(const vec3& a, float b) { return vec3(a.x / b, a.y / b, a.z / b); }


// mat3とvec3の乗算
vec3 operator*(const mat3& m, const vec3& v);

#endif // VEC3_H_INCLUDED