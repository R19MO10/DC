/**
* Quaternion.h
*/
#ifndef QUATERNION_H_INCLUDED
#define QUATERNION_H_INCLUDED

/**
* クォータニオン
*/
struct Quaternion
{
	float x, y, z, w;

	Quaternion() = default;
	explicit Quaternion(float s) : x(s), y(s), z(s), w(s) {}
	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

#endif // QUATERNION_H_INCLUDED