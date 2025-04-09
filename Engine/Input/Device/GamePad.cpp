#include "GamePad.h"

#include "../../Engine/DebugLog.h"
#include <GLFW/glfw3.h>
#undef APIENTRY
#include <Windows.h>
#undef max
#undef min
#include <Xinput.h>

#pragma comment(lib, "Xinput.lib")

struct Vibration {
	WORD leftMotorSpeed;  // �����[�^�[���x (0�`65535)
	WORD rightMotorSpeed; // �E���[�^�[���x (0�`65535)
};


void GamePad::UpdeteStates(float deltaSeconds)
{
	isConnected = false;

	// �O�t���[���̃{�^����Ԃ�ۑ�
	prevButtonStates.swap(buttonStates);
	buttonStates.clear();

	leftStick = vec2(0);
	rightStick = vec2(0);

	preTrigger = trigger;
	trigger = vec2(0);

	// �Q�[���p�b�g�̏�Ԃ��擾
	GLFWgamepadstate gamepadState;
	if (glfwGetGamepadState(0, &gamepadState)) {
		isConnected = true;

		// �e�{�^���̏��
		for (int bCode = GLFW_GAMEPAD_BUTTON_A; bCode <= GLFW_GAMEPAD_BUTTON_LAST; ++bCode) {
			if (gamepadState.buttons[bCode] == GLFW_PRESS) {
				buttonStates.insert(bCode);
			}
		}

		// ���X�e�B�b�N
		leftStick.x = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
		leftStick.y = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
		for (int i = 0; i < 2; ++i) {
			// �f�b�h�]�[���ȉ��Ȃ�l��0�ɂ���
			if (dot(leftStick[i]) <= dot(leftStickDeadZone)) {
				leftStick[i] = 0.0f;
			}
		}

		// �E�X�e�B�b�N
		rightStick.x = gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
		rightStick.y = gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
		for (int i = 0; i < 2; ++i) {
			// �f�b�h�]�[���ȉ��Ȃ�l��0�ɂ���
			if (dot(rightStick[i]) <= dot(rightStickDeadZone)) {
				rightStick[i] = 0.0f;
			}
		}

		// �g���K�[
		trigger.x = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
		trigger.y = gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
		// �g���K�[�l��0�`1�ɐ�������
		trigger += 1;
		trigger *= 0.5f;
	}

	// �o�C�u���[�V�����̃^�C�}�[�_�E��
	if (vibaTimer_s > 0) {
		vibaTimer_s -= deltaSeconds;

		// �^�C�}�[��0�ɂȂ�����o�C�u���[�V�������~�߂�
		if (vibaTimer_s <= 0) {
			XINPUT_STATE state;
			DWORD dwResult = XInputGetState(0, &state);
			if (dwResult == ERROR_SUCCESS) {
				Vibration vibration = { 0, 0 };

				// �o�C�u���[�V�������~
				XInputSetState(0, (XINPUT_VIBRATION*)&vibration);
				vibaPower = 0;
				vibaTimer_s = 0;
			}
		}
	}
}

/**
* �o�C�u���[�V���������s����
*
* @param power �U���̑傫��
* @param time  �U���̌p������
*/
void GamePad::Vibartion(float power, float time)
{
	if (power < 0) {
		return;
	}

	power = std::min(power, 100.0f);
	if (vibaPower > power) {
		// ���ݎ��s���̐U�����������������珈�����Ȃ�
		return;
	}

	XINPUT_STATE state;
	DWORD dwResult = XInputGetState(0, &state);
	if (dwResult == ERROR_SUCCESS) {
		constexpr float VibartionPower = 65535.0f / 100.0f;
		WORD wpow = static_cast<WORD>(VibartionPower * power);
		Vibration vibration = { wpow, wpow };

		// �o�C�u���[�V���������s
		XInputSetState(0, (XINPUT_VIBRATION*)&vibration);
		vibaPower = power;
		vibaTimer_s = time;
	}
}
