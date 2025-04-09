/**
* QuaternionHelper.h
*/
#ifndef QUATERNIONHELPER_H_INCLUDED
#define QUATERNIONHELPER_H_INCLUDED
#include "Quaternion.h"
#include <limits>
#include <cmath>

// ����
inline float dot(const Quaternion& a, const Quaternion& b) 
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

}

// �N�H�[�^�j�I�������ʐ��`��Ԃ���
inline Quaternion slerp(const Quaternion& a, const Quaternion& b, float t)
{
	// �N�H�[�^�j�I���̕�ԕ����͕�Ԏ��ɑ΂��Ď��v���A�܂��͔����v���ɂȂ�
	// �}�C�i�X�̃R�T�C���̒l�́A�����ɂȂ�������I������Ă��邱�Ƃ�����
		  // ��������ԕ����͏�ɒZ�������Ȃ̂ŁA�����𔽓]����
	Quaternion c = b;
	float cosTheta = dot(a, b);
	if (cosTheta < 0) {
		c = Quaternion(-c.x, -c.y, -c.z, -c.w);
		cosTheta = -cosTheta;
	}

	// cosTheta��1�ɋ߂��ꍇ�͉��Z�덷���o��̂Ő��`��Ԃő�p
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