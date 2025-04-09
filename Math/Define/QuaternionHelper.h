/**
* QuaternionHelper.h
*/
#ifndef QUATERNIONHELPER_H_INCLUDED
#define QUATERNIONHELPER_H_INCLUDED
#include "Quaternion.h"
#include <limits>
#include <cmath>

// 内積
inline float dot(const Quaternion& a, const Quaternion& b) 
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

}

// クォータニオンを球面線形補間する
inline Quaternion slerp(const Quaternion& a, const Quaternion& b, float t)
{
	// クォータニオンの補間方向は補間軸に対して時計回り、または反時計回りになる
	// マイナスのコサインの値は、遠回りになる方向が選択されていることを示す
		  // 正しい補間方向は常に短い方向なので、方向を反転する
	Quaternion c = b;
	float cosTheta = dot(a, b);
	if (cosTheta < 0) {
		c = Quaternion(-c.x, -c.y, -c.z, -c.w);
		cosTheta = -cosTheta;
	}

	// cosThetaが1に近い場合は演算誤差が出るので線形補間で代用
	float t0, t1;
	if (cosTheta > 1.0f - std::numeric_limits<float>::epsilon()) {
		t0 = 1 - t;
		t1 = t;
	}
	else {
		const float theta = acos(cosTheta);
		const float sinTheta = sin(theta);
		t0 = sin(theta * (1 - t)) / sinTheta;
		t1 = sin(theta * t) / sinTheta;
	}

	return Quaternion(
		a.x * t0 + c.x * t1,
		a.y * t0 + c.y * t1,
		a.z * t0 + c.z * t1,
		a.w * t0 + c.w * t1);
}

#endif // QUATERNIONHELPER_H_INCLUDED