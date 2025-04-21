/**
* @file Camera.h
*/
#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED
#include "../Engine/Component.h"
#include "../Math/MyMath.h"

/**
* �J�����R���|�[�l���g
*/
class Camera : public Component
{
public:
	// ��������p(�ʓx�@)
	float fovY = 1.047f;

	// ����p�ɂ��g�嗦�̋t��
	// x: ����p�ɂ�鐅���g�嗦
	// y: ����p�ɂ�鐂���g�嗦
	vec2 fovScale = vec2(0);;

	float near = 0.35f;   // �ߕ���
	float far = 1000.0f;  // ������

	/*�[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ȃp�����[�^A, B*/
	float A = 0.01f;      // ���ߖ@�p�����[�^A
	float B = 0.99f;      // ���ߖ@�p�����[�^B

private:
	float shake_elapsedTime_s = 0;	// �V�F�C�N�J�n����̌o�ߎ���
	float maxShakePower = 0;		// �V�F�C�N�̋���
	float currentShakePower = 0;	// ���݂̃V�F�C�N�̋���

public:
	bool isShake = false;	// �J�����V�F�C�N�̎��s�̗L��
	float shakeSpeed = 150;	// �V�F�C�N���x

private:
	mat4 matView = mat4(1);	// �r���[�s��
	mat4 matProj = mat4(1);	// �r���[�v���W�F�N�V�����s��

public:
	Camera() 
		:Component("Camera")
	{};
	virtual ~Camera() = default;

public:
	// �r���[�s����擾����
	inline const mat4& GetViewMatrix() const {
		return matView;
	}

	// �r���[�v���W�F�N�V�����s����擾����
	inline const mat4& GetProjectionMatrix() const {
		return matProj;
	}

public:
	// �J�����U�������s
	inline void CameraShake(float power) {
		if (power <= 0) {
			// �U���p���[��0�ȉ��Ȃ珈�����Ȃ�
			return;
		}

		isShake = true;

		power *= 0.001f;	//�K�؂ȐU�����x�܂Œ���

		// ���ݍs���Ă���U����������������K������
		if (power > currentShakePower) {
			currentShakePower = maxShakePower = power;
		}
	}

public:
	virtual void EndUpdate(
		float deltaSeconds, mat4& ownerWorldTransMat) override;
	virtual void DrawImGui() override;
};
using CameraPtr = std::shared_ptr<Camera>;

#endif // CAMERA_H_INCLUDED