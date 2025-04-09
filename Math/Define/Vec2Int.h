/**
* @file Vec2Int.h
*/
#ifndef VEC2INT_H_INCLUDED
#define VEC2INT_H_INCLUDED

// ��s�錾
struct vec2;

struct vec2Int {
	int x, y;

	constexpr vec2Int() : x(0), y(0) {}
	constexpr explicit vec2Int(int s) : x(s), y(s) {}
	constexpr vec2Int(int x, int y) : x(x), y(y) {}

	// �R�s�[�R���X�g���N�^
	constexpr vec2Int(const vec2Int& v) : x(v.x), y(v.y) {}

	// vec2����vec2Int�ւ̕ϊ��R���X�g���N�^
	explicit vec2Int(const vec2& v);

	// vec2Int�̒P��-���Z�q
	constexpr vec2Int operator-() { return vec2Int(-x, -y); }
	constexpr const vec2Int operator-() const { return vec2Int(-x, -y); }

	// �Y�����Z�q
	constexpr int& operator[](size_t n) { return *(&x + n); }
	constexpr const int& operator[](size_t n) const { return *(&x + n); }

	// ��r���Z�q
	constexpr bool operator==(const vec2Int& v) const {
		return (x == v.x) && (y == v.y);
	}
	constexpr bool operator!=(const vec2Int& v) const {
		return !(*this == v);
	}
	constexpr bool operator==(const int& v) const {
		return (x == v) && (y == v);
	}
	constexpr bool operator!=(const int& v) const {
		return !(*this == v);
	}

	// vec2Int���m
	constexpr vec2Int& operator= (const vec2Int& v) {
		if (this == &v) return *this; // ���ȑ���`�F�b�N
		x = v.x;
		y = v.y;
		return *this;
	}
	constexpr vec2Int& operator+=(const vec2Int& v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	constexpr vec2Int& operator-=(const vec2Int& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	constexpr vec2Int& operator*=(const vec2Int& v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	constexpr vec2Int& operator/=(const vec2Int& v) {
		x = static_cast<int>(x / v.x);
		y = static_cast<int>(y / v.y);
		return *this;
	}


	// vec2Int��int
	constexpr vec2Int& operator+=(const int& v) { return *this += vec2Int(v); }
	constexpr vec2Int& operator-=(const int& v) { return *this -= vec2Int(v); }
	constexpr vec2Int& operator*=(const int& v) { return *this *= vec2Int(v); }
	constexpr vec2Int& operator/=(const int& v) { return *this /= vec2Int(v); }
};

// vec2Int���m
constexpr vec2Int operator+(const vec2Int& a, const vec2Int& b) { return vec2Int(a.x + b.x, a.y + b.y); }
constexpr vec2Int operator-(const vec2Int& a, const vec2Int& b) { return vec2Int(a.x - b.x, a.y - b.y); }
constexpr vec2Int operator*(const vec2Int& a, const vec2Int& b) { return vec2Int(a.x * b.x, a.y * b.y); }
constexpr vec2Int operator/(const vec2Int& a, const vec2Int& b) {
	return vec2Int(static_cast<int>(a.x / b.x), static_cast<int>(a.y / b.y));
}


// vec2Int��int
constexpr vec2Int operator+(const vec2Int& a, int b) { return vec2Int(a.x + b, a.y + b); }
constexpr vec2Int operator+(int a, const vec2Int& b) { return b + a; }
constexpr vec2Int operator-(const vec2Int& a, int b) { return vec2Int(a.x - b, a.y - b); }
constexpr vec2Int operator-(int a, const vec2Int& b) { return vec2Int(a) - b; }
constexpr vec2Int operator*(const vec2Int& a, int b) { return vec2Int(a.x * b, a.y * b); }
constexpr vec2Int operator*(int a, const vec2Int& b) { return b * a; }
constexpr vec2Int operator/(const vec2Int& a, int b) {
	return vec2Int(static_cast<int>(a.x / b), static_cast<int>(a.y / b));
}

#endif // VEC2INT_H_INCLUDED