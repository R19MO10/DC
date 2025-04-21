/**
* @file Keyboard.h
*/
#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED
#include "IInput.h"
#include <unordered_set>

/**
* �L�[�{�[�h���͏�ԊǗ��N���X
*/
class Keyboard : public IInput
{
private:
	GLFWwindow* window = nullptr;	// �E�B���h�E�|�C���^

	std::unordered_set<int> keyStates;	// ���݃t���[���ɉ����ꂽ�L�[
	std::unordered_set<int> prevKeyStates;	// �O�t���[���ɉ����ꂽ�L�[

public:
	void StartUp(GLFWwindow* window) override;
	void UpdeteStates(float deltaSeconds) override;

	// �L�[���p���I�ɉ�����Ă��邩�擾
	inline bool GetKey(int keyCode) const noexcept {
		// ���݃t���[���ɉ�����Ă���
		return keyStates.contains(keyCode);
	}
	// �L�[�������ꂽ�u�Ԃ��ǂ����擾
	inline bool GetKeyDown(int keyCode) const noexcept {
		// ���݃t���[���ɉ�����Ă��āA�O�t���[���ɉ�����Ă��Ȃ�
		return (keyStates.contains(keyCode) && !prevKeyStates.contains(keyCode));
	}
	// �L�[�������ꂽ�u�Ԃ��ǂ����擾
	inline bool GetKeyUp(int keyCode) const noexcept {
		// ���݃t���[���ɉ�����Ă��炸�A�O�t���[���ɉ�����Ă���
		return (!keyStates.contains(keyCode) && prevKeyStates.contains(keyCode));
	}

	// �����ꂩ�̃L�[���p���I�ɉ�����Ă��邩�擾
	inline bool GetAnyKey() const noexcept {
		return (keyStates.size() > 0);
	}
	//  �����ꂩ�̃L�[�������ꂽ�u�Ԃ��ǂ����擾
	inline bool GetAnyKeyDown() const noexcept {
		for (const auto& bCode : keyStates) {
			if (!prevKeyStates.contains(bCode)) {
				return true;
			}
		}
		return false;
	}
	//  �����ꂩ�̃L�[�������ꂽ�u�Ԃ��ǂ����擾
	inline bool GetAnyKeyUp() const noexcept {
		for (const auto& bCode : prevKeyStates) {
			if (!keyStates.contains(bCode)) {
				return true;
			}
		}
		return false;
	}
};

#endif // KEYBOARD_H_INCLUDED