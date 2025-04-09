/**
* @file FreeRoamCamera.h
*/
#ifndef FREEROAMCAMERA_H_INCLUDED
#define FREEROAMCAMERA_H_INCLUDED
#include "../../Utility/MyGameFunction.h"
#include "../../Math/MyMath.h"
#include "../../Engine/Engine.h"
#include "../../Engine/Input/InputManager.h"


/**
* ���[���h�������R�ɔ�щ��J����
*/
class FreeRoamCamera
{
public:
	vec3 position = vec3(0);
	float yaw = 0, pitch = 0;
	vec3 front = vec3(0);

	mat4 transMat = mat4(1);

	float speed = 10;	// �ړ����x

	// ��������p(�ʓx�@)
	float fovY = 1.047f;

	// ����p�ɂ��g�嗦�̋t��
	// x: ����p�ɂ�鐅���g�嗦
	// y: ����p�ɂ�鐂���g�嗦
	vec2 fovScale = vec2(0);

	float near = 0.35f;   // �ߕ���
	float far = 1000.0f;  // ������

	/*�[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ȃp�����[�^A, B*/
	float A = 0.01f;      // ���ߖ@�p�����[�^A
	float B = 0.99f;      // ���ߖ@�p�����[�^B
	
private:
	mat4 matView = mat4(1);	// �r���[�s��
	mat4 matProj = mat4(1);	// �r���[�v���W�F�N�V�����s��

public:
	FreeRoamCamera(const vec3 position, const vec3 rotation) 
		:position(position), yaw(rotation.y), pitch(rotation.x)
	{
		// �J������pitch������≺���ɒB���Ȃ��悤�ɐ�������
		pitch = std::clamp(pitch, -1.55f, 1.55f);

		front = {
			std::cos(yaw) * std::cos(pitch),
			std::sin(pitch),
			std::sin(yaw) * std::cos(pitch)
		};
		front = normalize(front);

		transMat = 
			TransformMatrix(position, vec3(pitch, yaw, 0), vec3(1));
	}
	virtual ~FreeRoamCamera() = default;

	// �r���[�s����擾����
	inline const mat4& GetViewMatrix() const {
		return matView;
	}

	// �r���[�v���W�F�N�V�����s����擾����
	inline const mat4& GetProjectionMatrix() const {
		return matProj;
	}

	void Update(float deltaSeconds, const Engine& engine)
	{
		// �ړ�����
		if (engine.FocusWindow()) {
			Mouse* mouse = InputManager::GetMouse();

			// �E�{�^���������Ă���Ԃ�������
			if (mouse->GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)) {

				// �}�E�X�̈ړ��ʂ��擾
				vec2 inputValue = mouse->GetDeltaMousePos();
				yaw -= inputValue.x;
				pitch -= inputValue.y;

				// �J������pitch������≺���ɒB���Ȃ��悤�ɐ�������
				pitch = std::clamp(pitch, -1.55f, 1.55f);

				front = {
					-std::cos(pitch) * std::sin(yaw),
					 std::sin(pitch),
					-std::cos(pitch) * std::cos(yaw)
				};
				front = normalize(front);


				Keyboard* keyBoard = InputManager::GetKeyboard();
				// ����p��ύX
				if (keyBoard->GetKey(GLFW_KEY_Z)) {
					fovY += deltaSeconds;
					fovY = std::min(fovY, 2.5f);
				}
				if (keyBoard->GetKey(GLFW_KEY_C)) {
					fovY -= deltaSeconds;
					fovY = std::max(fovY, 0.1f);
				}
				if (keyBoard->GetKey(GLFW_KEY_X)) {
					fovY = 1.047f;
				}
				// ���x��ύX
				if (keyBoard->GetKey(GLFW_KEY_1)) {
					speed += 15 * deltaSeconds;
				}
				if (keyBoard->GetKey(GLFW_KEY_3)) {
					speed -= 15 * deltaSeconds;
					speed = std::max(speed, 0.5f);
				}

				vec3 moveVector;
				// �O�i
				if (keyBoard->GetKey(GLFW_KEY_W)) {
					moveVector += front;
				}
				// ���
				if (keyBoard->GetKey(GLFW_KEY_S)) {
					moveVector -= front;
				}
				// ���Ɉړ�
				if (keyBoard->GetKey(GLFW_KEY_A)) {
					moveVector -= normalize(cross(front, vec3(0, 1, 0)));
				}
				// �E�Ɉړ�
				if (keyBoard->GetKey(GLFW_KEY_D)) {
					moveVector += normalize(cross(front, vec3(0, 1, 0)));
				}

				position += moveVector * speed * deltaSeconds;

				// ��Ɉړ�
				if (keyBoard->GetKey(GLFW_KEY_E)) {
					position.y += 5 * deltaSeconds;
				}
				// ���Ɉړ�
				if (keyBoard->GetKey(GLFW_KEY_Q)) {
					position.y -= 5 * deltaSeconds;
				}

				transMat = TransformMatrix(position, vec3(pitch, yaw, 0), vec3(1));
			}
		}
		
		// ���ߖ@�p�����[�^A, B���v�Z
		// �i�[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ȃp�����[�^A, B�j
		/* �p�[�X�y�N�e�B�u�s��̋t�̌v�Z���s���A�r���[���W�n�ɕϊ������ */
		A = -2 * far * near / (far - near);
		B = (far + near) / (far - near);

		// ����p�ɂ��g�嗦���v�Z
		fovScale.y =	// ����
			1 / tan(fovY / 2);
		fovScale.x =	//����
			fovScale.y * (1 / engine.GetAspectRatio());

		// �r���[�s����쐬
		const vec3 eye = position;
		const vec3 target = position * front;
		matView = LookAt(eye, target, vec3(0, 1, 0));

		// �r���[�v���W�F�N�V�����s����쐬
		matProj = Perspective(fovY, engine.GetAspectRatio(), near, far);
	}
};

#endif // FREEROAMCAMERA_H_INCLUDED