/**
* Mat3Helper.h
*/
#ifndef MAT3HELPER_H_INCLUDED
#define MAT3HELPER_H_INCLUDED
#include "Mat3Math.h"

/**
* ���`���
*
* @param m1 1�߂̍s��
* @param m2 2�߂̍s��
* @param ratio ��������(0.0�Fm1���Ԃ����, 1.0�Fm2���Ԃ����)
*
* @return ���`��Ԃ��ꂽ�s��
*/
inline mat3 lerp(const mat3& m1, const mat3& m2, float ratio) {
	return m1 * (1.0f - ratio) + m2 * ratio;
}

// �]�u�s��
inline mat3 transpose(const mat3& m)
	{
		mat3 t;

		t[0][0] = m[0][0];
		t[0][1] = m[1][0];
		t[0][2] = m[2][0];

		t[1][0] = m[0][1];
		t[1][1] = m[1][1];
		t[1][2] = m[2][1];

		t[2][0] = m[0][2];
		t[2][1] = m[1][2];
		t[2][2] = m[2][2];

		return t;
	}

// �t�s��
inline mat3 inverse(const mat3& m)
{
    // �]���q�s����v�Z
    mat3 adjugate;
    adjugate[0][0] = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    adjugate[0][1] = -m[0][1] * m[2][2] + m[0][2] * m[2][1];
    adjugate[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];

    adjugate[1][0] = -m[1][0] * m[2][2] + m[1][2] * m[2][0];
    adjugate[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
    adjugate[1][2] = -m[0][0] * m[1][2] + m[0][2] * m[1][0];

    adjugate[2][0] = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    adjugate[2][1] = -m[0][0] * m[2][1] + m[0][1] * m[2][0];
    adjugate[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];

    // �s�񎮂��v�Z
    const float det =
        m[0][0] * adjugate[0][0] + m[0][1] * adjugate[1][0] + m[0][2] * adjugate[2][0];

    // �s�񎮂̋t�����|����
    const float invDet = 1.0f / det;
    adjugate[0] *= invDet;
    adjugate[1] *= invDet;
    adjugate[2] *= invDet;

    return adjugate;
}

#endif // MAT3HELPER_H_INCLUDED