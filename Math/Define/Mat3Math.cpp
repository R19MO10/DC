/**
* @ Mat3Math.cpp
*/
#include "Mat3Math.h"

#include "Mat4Math.h"
#include "Quaternion.h"

// mat4‚©‚çmat3‚Ö‚Ì•ÏŠ·
mat3::mat3(const mat4& m) 
{
	data[0] = vec3(m[0]);
	data[1] = vec3(m[1]);
	data[2] = vec3(m[2]);
}

// Quaternion‚©‚çmat3‚Ö‚Ì•ÏŠ·
mat3::mat3(const Quaternion& q)
{
	const float xx(q.x * q.x);
	const float yy(q.y * q.y);
	const float zz(q.z * q.z);
	const float xz(q.x * q.z);
	const float xy(q.x * q.y);
	const float yz(q.y * q.z);
	const float xw(q.w * q.x);
	const float yw(q.w * q.y);
	const float zw(q.w * q.z);

	data[0] = { 1 - 2 * (yy + zz), 2 * (xy + zw),     2 * (xz - yw) };
	data[1] = { 2 * (xy - zw),     1 - 2 * (xx + zz), 2 * (yz + xw) };
	data[2] = { 2 * (xz + yw),     2 * (yz - xw),     1 - 2 * (xx + yy) };
}
