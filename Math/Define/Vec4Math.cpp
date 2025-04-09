/**
* @ Vec4Math.cpp
*/
#include "Vec4Math.h"

#include "Vec2Math.h"
#include "Vec3Math.h"
#include "Mat4Math.h"

vec4::vec4(const vec2& xy, float z, float w)
	: x(xy.x), y(xy.y), z(z), w(w) 
{}

vec4::vec4(const vec3& xyz, float w)
	: x(xyz.x), y(xyz.y), z(xyz.z), w(w) 
{}


// vec4‚Ævec2
vec4& vec4::operator= (const vec2& v)
{
	x = v.x;
	y = v.y;
	return *this;
}

vec4& vec4::operator+= (const vec2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

vec4& vec4::operator-= (const vec2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

vec4& vec4::operator*= (const vec2& v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}

vec4& vec4::operator/= (const vec2& v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}


// vec4‚Ævec3
vec4& vec4::operator= (const vec3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

vec4& vec4::operator+= (const vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}
vec4& vec4::operator-= (const vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}
vec4& vec4::operator*= (const vec3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}
vec4& vec4::operator/= (const vec3& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}


// mat4‚Ævec4
vec4 operator*(const mat4& m, const vec4& v)
{
	return m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
}