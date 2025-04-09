/**
* @file MyMath.h
*/
#ifndef MYMATH_H_INCLUDED
#define MYMATH_H_INCLUDED

// �p�x�֌W
#include "Define/AngleMath.h"

// Vector�^
#include "Define/Vec2Math.h"
#include "Define/Vec3Math.h"
#include "Define/Vec4Math.h"

// Vector�^�v�Z�⏕
#include "Define/Vec2Helper.h"
#include "Define/Vec3Helper.h"
#include "Define/Vec4Helper.h"

// VectorInt�^
#include "Define/Vec2Int.h"

// Matrix�^
#include "Define/Mat3Math.h"
#include "Define/Mat4Math.h"

// Matrix�^�v�Z�⏕
#include "Define/Mat3Helper.h"
#include "Define/Mat4Helper.h"

// Quaternion�^
#include "Define/Quaternion.h"

// Quaternion�^�v�Z�⏕
#include "Define/QuaternionHelper.h"


// �Q��
inline float dot(float f) {
	return f * f;
}

// ���`���
inline float lerp(const float& start, const float& end, float alpha) {
	return (1.0f - alpha) * start + alpha * end;
}

namespace Mat {
	
	// ���s�ړ��s����쐬
	inline mat4 Translate(const vec3& v)
	{
		mat4 m(1);
		m[3][0] = v.x;
		m[3][1] = v.y;
		m[3][2] = v.z;
		return m;
	}

	// �g��k���s����쐬
	inline mat4 Scale(const vec3& v)
	{
		mat4 m(1);
		m[0][0] = v.x;
		m[1][1] = v.y;
		m[2][2] = v.z;
		return m;
	}
}

// ��]�p�x�����]�s����쐬
inline mat3 RotationMatrix(const vec3& rotation)
{
	// X����]
	const float sinX = sin(rotation.x);
	const float cosX = cos(rotation.x);
	mat3 mx = {
		{ 1,		0,			0	  },
		{ 0,		cosX,		sinX  },
		{ 0,		-sinX,		cosX  }
	};

	// Y����]
	const float sinY = sin(rotation.y);
	const float cosY = cos(rotation.y);
	mat3 my = {
		{ cosY,		0,			-sinY },
		{ 0,		1,			0	  },
		{ sinY,		0,			cosY  }
	};

	// Z����]
	const float sinZ = sin(rotation.z);
	const float cosZ = cos(rotation.z);
	mat3 mz = {
		{ cosZ,		sinZ,		0	  },
		{ -sinZ,	cosZ,		0	  },
		{ 0,		0,			1	  }
	};

	return my * mx * mz;
}

// ���s�ړ��ʁA��]�p�x�A�g�嗦������W�ϊ��s����쐬
inline mat4 TransformMatrix(
	const vec3& position, const vec3& rotation, const vec3& scale)
{
	const mat3 m = RotationMatrix(rotation);
	return {
		{ m.data[0] * scale.x,	0 },
		{ m.data[1] * scale.y,	0 },
		{ m.data[2] * scale.z,	0 },
		{ position,				1 }
	};
}
inline mat4 TransformMatrix(
	const vec3& position, const mat3& rotation, const vec3& scale)
{
	return {
		{ rotation.data[0] * scale.x,	0 },
		{ rotation.data[1] * scale.y,	0 },
		{ rotation.data[2] * scale.z,	0 },
		{ position,						1 }
	};
}

// 4x4�s�񂩂�g�嗦�𒊏o����
inline vec3 ExtractScale(const mat4& transformMatrix)
{
	const float sx = length(vec3(transformMatrix[0]));
	const float sy = length(vec3(transformMatrix[1]));
	const float sz = length(vec3(transformMatrix[2]));

	return { sx, sy, sz };
}

// ��]�s�񂩂�I�C���[�p�𒊏o����
inline vec3 ExtractRotation(const mat3& rotationMatrix)
{
	const float yaw =
		atan2(rotationMatrix.data[2][0], rotationMatrix.data[2][2]);

	const float pitch =
		atan2(-rotationMatrix.data[2][1],
		sqrt(rotationMatrix.data[0][1] * rotationMatrix.data[0][1] + 
			rotationMatrix.data[1][1] * rotationMatrix.data[1][1]));

	const float s = std::sin(yaw);
	const float c = std::cos(yaw);
	
	const float roll = 
		atan2(s * rotationMatrix.data[1][2] - c * rotationMatrix.data[1][0],
			c * rotationMatrix.data[0][0] - s * rotationMatrix.data[0][2]);

	return { pitch, yaw, roll };
}

/**
* ���W�ϊ��s��𕽍s�ړ��A�g�嗦�A��]�s��̊e�����ɕ�������
*
* @param[in]  transform �������̍��W�ϊ��s��
* @param[out] translate ���s�ړ��̊i�[��ƂȂ�ϐ�
* @param[out] rotation  ��]���̊i�[��ƂȂ�ϐ�
* @param[out] scale     �g�嗦�̊i�[��ƂȂ�ϐ�
*/
inline void Decompose(const mat4& transform,
	vec3& translate, vec3& rotation, vec3& scale)
{
	// ���s�ړ��𕪉�
	translate = vec3(transform[3]);

	// �g�嗦�𕪉�
	scale = ExtractScale(transform);

	// ��]�s��𕪉�
	mat3 r = mat3(transform);
	r[0] /= scale.x;
	r[1] /= scale.y;
	r[2] /= scale.z;
	rotation = ExtractRotation(r);
}
inline void Decompose(const mat4& transform,
	vec3& translate, mat3& rotation, vec3& scale)
{
	// ���s�ړ��𕪉�
	translate = vec3(transform[3]);

	// �g�嗦�𕪉�
	scale = ExtractScale(transform);

	// ��]�s��𕪉�
	rotation = mat3(transform);
	rotation[0] /= scale.x;
	rotation[1] /= scale.y;
	rotation[2] /= scale.z;
}

/**
* �����_���v�Z����
* 
* @param ���_�̍��W
* @param ���_�̉�]
* 
* @return �����_���W
*/
inline vec3 CalcTargetPosition(const vec3& position, const vec3& rotation) {
	// �J�����̕����x�N�g���𐶐�����i��Ƃ��Ċȗ����j
	vec3 direction(
		cos(rotation.x) * sin(rotation.y),
		sin(rotation.x),
		cos(rotation.x) * cos(rotation.y)
	);

	// �J�����̒����_�͈ʒu�x�N�g���ɕ����x�N�g�������Z�������̂Ƃ��Čv�Z�����
	vec3 target = position + direction;

	return target;
}

/**
* �r���[�s����쐬����
*
* @param eye    ���_�̍��W
* @param target �����_�̍��W
* @param up     ���_�̏�������w�����̃x�N�g��
*
* @return eye, target, up����쐬�����r���[�s��
*/
inline mat4 LookAt(const vec3& eye, const vec3& target, const vec3& up)
{
	// ���[���h���W�n�ɂ����鎋�_���W�n��XYZ���̌������v�Z
	const vec3 axisZ = normalize(eye - target);
	const vec3 axisX = normalize(cross(up, axisZ));
	const vec3 axisY = normalize(cross(axisZ, axisX));

	// ���W�����x�N�g���Ɏˉe����悤�ɍs���ݒ�
	mat4 m;
	m[0] = { axisX.x, axisY.x, axisZ.x, 0 };
	m[1] = { axisX.y, axisY.y, axisZ.y, 0 };
	m[2] = { axisX.z, axisY.z, axisZ.z, 0 };

	// eye�����_�ƂȂ�悤�ɁAeye���e���Ɏˉe���ĕ��s�ړ��ʂ��v�Z
	m[3] = { -dot(axisX, eye), -dot(axisY, eye), -dot(axisZ, eye), 1 };

	return m;
}


/**
* ���s���e�s��
*
* @param left	�`��͈͂̍��[�܂ł̋���
* @param right	�`��͈͂̉E�[�܂ł̋���
* @param bottom	�`��͈͂̉��[�܂ł̋���
* @param top	�`��͈͂̏�[�܂ł̋���
* @param zNear	�`��͈͂Ɋ܂܂��ŏ�Z���W
* @param zFar	�`��͈͂Ɋ܂܂��ő�Z���W
*/
inline mat4 Orthogonal(float left, float right, float bottom, float top, float zNear, float zFar)
{

	 // �`��͈͂̃T�C�Y�ƒ��S���W���v�Z
	const vec3 size = { right - left, top - bottom, zFar - zNear };
	const vec3 center = { right + left, top + bottom, zFar + zNear };

	// �`��͈͓��̍��W��-1�`+1�ɕϊ������悤�ɁA�g�嗦�ƕ��s�ړ���ݒ�
	/* ���W�Ɂu2 / �`��͈́v���|���ăN���b�v���W�n�֕ϊ��i�N���b�v���W�n��xYZ���}1�̍��W�n�Ȃ��߁j*/
	mat4 m;
	m[0] = { 2 / size.x,    0,				0,				0 };
	m[1] = { 0,				2 / size.y,     0,				0 };
	m[2] = { 0,				0,				-2 / size.z,	0 };	/*Z���𔽓]�iOpenGL�̎��_���W�n�͉E��n�A�N���b�v���W�n�͍���n�Ȃ��߁j*/
	m[3] = vec4(-center / size, 1);

	return m;
}

/**
* �������e�s��
*
* @param fovy_rad		��������p(���W�A��)
* @param aspectRatio	��������p�ɑ΂��鐅������p�̔䗦
* @param zNear			�`��͈͂Ɋ܂܂��ŏ�Z���W
* @param zFar			�`��͈͂Ɋ܂܂��ő�Z���W
*/
inline mat4 Perspective(float fovy_rad, float aspectRatio, float zNear, float zFar)
{
	const float tanHalfFovy = tan(fovy_rad * 0.5f);

	mat4 Result(0);
	Result[0][0] = 1 / (aspectRatio * tanHalfFovy);
	Result[1][1] = 1 / (tanHalfFovy);
	Result[2][2] = -(zFar + zNear) / (zFar - zNear);
	Result[2][3] = -1;
	Result[3][2] = -(2 * zFar * zNear) / (zFar - zNear);

	return Result;
}

#endif // MYMATH_H_INCLUDED