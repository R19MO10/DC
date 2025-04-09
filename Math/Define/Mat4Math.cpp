/**
* @ Mat4Math.cpp
*/
#include "Mat4Math.h"

#include "Mat3Math.h"
#include "Quaternion.h"

mat4::mat4(const mat3& m)
{
	data[0] = vec4(m[0], 0);
	data[1] = vec4(m[1], 0);
	data[2] = vec4(m[2], 0);
	data[3] = vec4(0, 0, 0, 1);
}

// Quaternion‚©‚çmat4‚Ö‚Ì•ÏŠ·
mat4::mat4(const Quaternion& q)
{
	mat4 m = mat4(mat3(q));
	data[0] = m[0];
	data[1] = m[1];
	data[2] = m[2];
	data[3] = m[3];
}

// mat4‚Æmat3
mat4& mat4::operator=(const mat3& m)
{
	for (int i = 0; i < 3; i++) {
		data[i] = m[i];
	}
	return *this;
}
mat4& mat4::operator+=(const mat3& m)
{
	for (int i = 0; i < 3; i++) {
		data[i] += m[i];
	}
	return *this;
}
mat4& mat4::operator-=(const mat3& m)
{
	for (int i = 0; i < 3; i++) {
		data[i] -= m[i];
	}
	return *this;
}
mat4& mat4::operator*=(const mat3& m)
{
	for (int i = 0; i < 3; i++) {
		data[i] *= m[i];
	}
	return *this;
}
