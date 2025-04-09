/**
* @ Vec3Math.cpp
*/
#include "Vec3Math.h"

#include "Vec2Math.h"
#include "Vec4Math.h"
#include "Mat3Math.h"

vec3::vec3(const vec2& xy, float z)
	: x(xy.x), y(xy.y), z(z) 
{}


// vec4からvec3への変換コンストラクタ
vec3::vec3(const vec4& v)
	: x(v.x), y(v.y), z(v.z)
{}


// vec3とvec2
vec3& vec3::operator= (const vec2& v)
{
	x = v.x;
	y = v.y;
	return *this;
}
vec3& vec3::operator+= (const vec2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}
vec3& vec3::operator-= (const vec2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}
vec3& vec3::operator*= (const vec2& v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}
vec3& vec3::operator/= (const vec2& v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}


// mat3とvec3
vec3 operator*(const mat3& m, const vec3& v)
{
	return m[0] * v.x + m[1] * v.y + m[2] * v.z;
}