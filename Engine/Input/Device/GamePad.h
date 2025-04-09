/**
* @file GamePad.h
*/
#ifndef GAMEPAD_H_INCLUDED
#define GAMEPAD_H_INCLUDED
#include"IInput.h"
#include "../../Math/MyMath.h"
#include <unordered_set>

/**
* �}�E�X���͏�ԊǗ�
*/
class GamePad : public IInput
{
private:
	bool isConnected = false;

	std::unordered_set<int> buttonStates;		// ���t���[���̉����ꂽ�{�^��
	std::unordered_set<int> prevButtonStates;	// �O�t���[���̉����ꂽ�{�^��

	vec2 leftStick = vec2(0);
	vec2 rightStick = vec2(0);

	vec2 trigger = vec2(0);
	vec2 preTrigger = vec2(0);

	float vibaPower = 0;	// �o�C�u���[�V�����p���[
	float vibaTimer_s = 0;	// �o�C�u���[�V������������

public:
	float leftStickDeadZone = 0.1f;		// �X�e�B�b�N�f�b�h�]�[��
	float rightStickDeadZone = 0.1f;

	float stickSensitivity = 0.1f;	// �X�e�B�b�N���x

public:
	void StartUp(GLFWwindow* window) override {}
	void UpdeteStates(float deltaSeconds) override;

	// �Q�[���p�b�g���ڑ�����Ă��邩�擾
	inline bool GetConnectionState() { return isConnected; }

	// �{�^�����p���I�ɉ�����Ă��邩�擾
	inline bool GetButton(int buttonCode) const noexcept {
		// ���݃t���[���ɉ�����Ă���
		return buttonStates.contains(buttonCode);
	}
	// �{�^���������ꂽ�u�Ԃ��ǂ����擾
	inline bool GetButtonDown(int buttonCode) const noexcept {
		// ���݃t���[���ɉ�����Ă��āA�O�t���[���ɉ�����Ă��Ȃ�
		return (buttonStates.contains(buttonCode) && !prevButtonStates.contains(buttonCode));
	}
	// �{�^���������ꂽ�u�Ԃ��ǂ����擾
	inline bool GetButtonUp(int buttonCode) const noexcept {
		// ���݃t���[���ɉ�����Ă��炸�A�O�t���[���ɉ�����Ă���
		return (!buttonStates.contains(buttonCode) && prevButtonStates.contains(buttonCode));
	}

	// �����ꂩ�̃{�^�����p���I�ɉ�����Ă��邩�擾
	inline bool GetAnyButton() const noexcept {
		return (buttonStates.size() > 0);
	}
	// �����ꂩ�̃{�^���������ꂽ�u�Ԃ��擾
	inline bool GetAnyButtonDown() const noexcept {
		for (const auto& bCode : buttonStates) {
			if (!prevButtonStates.contains(bCode)) {
				return true;
			}
		}
		return false;
	}
	// �����ꂩ�̃{�^���������ꂽ�u�Ԃ��擾
	inline bool GetAnyButtonUp() const noexcept {
		for (const auto& bCode : prevButtonStates) {
			if (!buttonStates.contains(bCode)) {
				return true;
			}
		}
		return false;
	}

	// ���X�e�B�b�N�̒l���擾
	inline vec2 GetLeftStick() const noexcept { return leftStick; }
	// �E�X�e�B�b�N�̒l���擾
	inline vec2 GetRightStick() const noexcept { return rightStick; }

	// ���g���K�[�̒l���擾
	inline float GetLeftTrigger() const noexcept { return trigger.x; }
	// ���g���K�[��������ďu�Ԃ��ǂ����擾
	inline bool GetLeftTriggerDown() const noexcept { 
		return (preTrigger.x == 0.0f && trigger.x > 0);
	}
	// ���g���K�[�������ꂽ�u�Ԃ��ǂ����擾
	inline bool GetLeftTriggerUp() const noexcept { 
		return (preTrigger.x != 0.0f && trigger.x == 0);
	}

	// �E�g���K�[�̒l���擾
	inline float GetRightTrigger() const noexcept { return trigger.y; }
	// �E�g���K�[��������ďu�Ԃ��ǂ����擾
	inline bool GetRightTriggerDown() const noexcept {
		return (preTrigger.y == 0.0f && trigger.y > 0);
	}
	// �E�g���K�[�������ꂽ�u�Ԃ��ǂ����擾
	inline bool GetRightTriggerUp() const noexcept {
		return (preTrigger.y != 0.0f && trigger.y == 0);
	}

	/**
	* �o�C�u���[�V���������s����
	*
	* @param power �U���̑傫��
	* @param time  �U���̌p������
	*/
	void Vibartion(float power, float time = 0.1f);
};

#endif // MOUSE_H_INCLUDED