/**
* @file Camera.cpp
*/
#include "Camera.h"

#include "../Utility/Random.h"

#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"
#include "../Engine/Input/InputManager.h"


void Camera::EndUpdate(
	float deltaSeconds, mat4& ownerWorldTransMat)
{
	// ���ߖ@�p�����[�^A, B���v�Z
	// �i�[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ȃp�����[�^A, B�j
	/* �p�[�X�y�N�e�B�u�s��̋t�̌v�Z���s���A�r���[���W�n�ɕϊ������ */
	A = -2 * far * near / (far - near);
	B = (far + near) / (far - near);

	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	// ����p�ɂ��g�嗦���v�Z
	fovScale.y =	// ����
		1 / tan(fovY / 2);			
	fovScale.x =	//����
		fovScale.y * (1 / engine->GetAspectRatio());	

	vec3 position;
	mat3 rotationMat;
	vec3 scale;
	Decompose(
		ownerWorldTransMat, position, rotationMat, scale);

	// �V�F�C�N����
	if (isShake) {
		shake_elapsedTime_s += deltaSeconds * shakeSpeed;

		// �V�F�C�N���W���v�Z
		/* �c�U���݂̂��N�����Ă��� */
		vec3 sPos = {
			0, std::sin(shake_elapsedTime_s) * currentShakePower, 0
		};

		// �U����K��
		position += rotationMat * sPos;

		// �U���̋��������񂾂񉺂��Ă���
		currentShakePower -= deltaSeconds * maxShakePower * 2;

		// �U���̋������Ȃ��Ȃ�����U�����I���
		if (currentShakePower < 0) {
			shake_elapsedTime_s = 0;
			isShake = false;
		}
	}

	// �r���[�s����쐬
	const vec3 eye = position;
	const vec3 target =
		position * rotationMat[2];
	matView = LookAt(eye, target, vec3(0, 1, 0));

	// �r���[�v���W�F�N�V�����s����쐬
	matProj =
		Perspective(fovY, engine->GetAspectRatio(), near, far);

	// �Ō�ɐU����K�����[���h���W�ϊ��s����㏑������
	ownerWorldTransMat =
		TransformMatrix(position, rotationMat, scale);
}

void Camera::DrawImGui()
{
	ImGui::DragFloat("Fov Y", &fovY);
	ImGui::Spacing();
	ImGui::DragFloat("Near", &near);
	ImGui::DragFloat("Far", &far);
	ImGui::Spacing();
	ImGui::DragFloat("A", &A);
	ImGui::DragFloat("B", &B);
	ImGui::Spacing();

	// �J�����V�F�C�N��C�ӎ��s
	static float sPwr = 20;
	ImGui::DragFloat("ShakeSpeed", &shakeSpeed);
	ImGui::DragFloat("ShakePower", &sPwr);
	if (ImGui::Button("CameraShake!!")) {
		isShake = true;
		currentShakePower = maxShakePower = sPwr * 0.001f;
	}
}