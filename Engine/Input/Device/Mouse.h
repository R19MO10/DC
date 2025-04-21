/**
* @file Mouse.h
*/
#ifndef MOUSE_H_INCLUDED
#define MOUSE_H_INCLUDED
#include "IInput.h"
#include "../../Math/MyMath.h"
#include <unordered_set>

/**
* �}�E�X���͏�ԊǗ�
*/
class Mouse : public IInput
{
private:
	GLFWwindow* window = nullptr;	// �E�B���h�E�|�C���^

	std::unordered_set<int> buttonStates;		// ���t���[���̉����ꂽ�{�^��
	std::unordered_set<int> prevButtonStates;	// �O�t���[���̉����ꂽ�{�^��

	vec2 mousePos = vec2(0);		// ���t���[���̃}�E�X�̍��W
	vec2 prevMousePos = vec2(0);	// �O�t���[���̃}�E�X���W

public:
	float mouseSensitivity = 1;	// �}�E�X���x

public:
	void StartUp(GLFWwindow* window) override;
	void UpdeteStates(float deltaSeconds) override;

	// �{�^�����p���I�ɉ�����Ă��邩�擾
	inline bool GetMouseButton(int buttonCode) const noexcept {
		// ���݃t���[���ɉ�����Ă���
		return buttonStates.contains(buttonCode);
	}
	// �{�^���������ꂽ�u�Ԃ��ǂ���
	inline bool GetMouseButtonDown(int buttonCode) const noexcept {
		// ���݃t���[���ɉ�����Ă��āA�O�t���[���ɉ�����Ă��Ȃ��擾
		return (buttonStates.contains(buttonCode) && !prevButtonStates.contains(buttonCode));
	}
	// �{�^���������ꂽ�u�Ԃ��ǂ���
	inline bool GetMouseButtonUp(int buttonCode) const noexcept {
		// ���݃t���[���ɉ�����Ă��炸�A�O�t���[���ɉ�����Ă����擾
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

	// �}�E�X�J�[�\���̍��W(�J�������W�n(Z=-1))���擾
	inline vec2 GetMousePosition() const noexcept { return mousePos; }
	// �}�E�X�̑��Έړ��ʂ��擾
	inline vec2 GetDeltaMousePos() const noexcept {
		return {
			mousePos.x - prevMousePos.x,
			prevMousePos.y - mousePos.y,
		};
	}
};

#endif // MOUSE_H_INCLUDED