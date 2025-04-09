/**
* @file Vec2Math.h
*/
#ifndef VEC2MATH_H_INCLUDED
#define VEC2MATH_H_INCLUDED

//先行宣言
struct vec2Int;
struct vec3;

struct vec2 {
	float x, y;

	constexpr vec2() : x(0), y(0) {}
	constexpr explicit vec2(float s) : x(s), y(s) {}
	constexpr vec2(float x, float y) : x(x), y(y) {}

	// コピーコンストラクタ
	constexpr vec2(const vec2& v) : x(v.x), y(v.y) {}

	// vec2Intからvec2への変換コンストラクタ
	explicit vec2(const vec2Int& v);

	// vec3からvec2への変換コンストラクタ
	explicit vec2(const vec3& v);

	// vec2の単項-演算子
	constexpr vec2 operator-() { return vec2(-x, -y); }
	constexpr const vec2 operator-() const { return vec2(-x, -y); }

	// 添字演算子
	constexpr float& operator[](size_t n) { return *(&x + n); }
	constexpr const float& operator[](size_t n) const { return *(&x + n); }

	// 比較演算子
	constexpr bool operator==(const vec2& v) const {
		return (x == v.x) && (y == v.y);
	}
	constexpr bool operator!=(const vec2& v) const {
		return !(*this == v);
	}
	constexpr bool operator==(const float& v) const {
		return (x == v) && (y == v);
	}
	constexpr bool operator!=(const float& v) const {
		return !(*this == v);
	}

	// vec2同士
	constexpr vec2& operator= (const vec2& v) {
		if (this == &v) return *this; // 自己代入チェック
		x = v.x;
		y = v.y;
		return *this;
	}
	constexpr vec2& operator+=(const vec2& v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	constexpr vec2& operator-=(const vec2& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	constexpr vec2& operator*=(const vec2& v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	constexpr vec2& operator/=(const vec2& v) {
		x /= v.x;
		y /= v.y;
		return *this;
	}


	// vec2とfloat
	constexpr vec2& operator+=(const float& v) { return *this += vec2(v); }
	constexpr vec2& operator-=(const float& v) { return *this -= vec2(v); }
	constexpr vec2& operator*=(const float& v) { return *this *= vec2(v); }
	constexpr vec2& operator/=(const float& v) { return *this /= vec2(v); }
};

// vec2同士
constexpr vec2 operator+(const vec2& a, const vec2& b) { return vec2(a.x + b.x, a.y + b.y); }
constexpr vec2 operator-(const vec2& a, const vec2& b) { return vec2(a.x - b.x, a.y - b.y); }
constexpr vec2 operator*(const vec2& a, const vec2& b) { return vec2(a.x * b.x, a.y * b.y); }
constexpr vec2 operator/(const vec2& a, const vec2& b) { return vec2(a.x / b.x, a.y / b.y); }


// vec2とfloat
constexpr vec2 operator+(const vec2& a, float b) { return vec2(a.x + b, a.y + b); }
constexpr vec2 operator+(float a, const vec2& b) { return b + a; }
constexpr vec2 operator-(const vec2& a, float b) { return vec2(a.x - b, a.y - b); }
constexpr vec2 operator-(float a, const vec2& b) { return vec2(a) - b; }
constexpr vec2 operator*(const vec2& a, float b) { return vec2(a.x * b, a.y * b); }
constexpr vec2 operator*(float a, const vec2& b) { return b * a; }
constexpr vec2 operator/(const vec2& a, float b) { return vec2(a.x / b, a.y / b); }

#endif // VEC2MATH_H_INCLUDED