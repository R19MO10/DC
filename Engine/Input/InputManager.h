/**
* @file InputManager.h
*/
#ifndef INPUTMANAGER_H_INCLUDED
#define INPUTMANAGER_H_INCLUDED
#include "../../Utility/Singleton.h"
#include "Device/Keyborad.h"
#include "Device/Mouse.h"
#include "Device/GamePad.h"


/**
* �C���v�b�g���Ǘ�����
*/
class InputManager : public Singleton<InputManager>
{
	friend class Engine;

private:
	Keyboard keyboard;
	Mouse mouse;
	GamePad gamePad;

public:
	InputManager() = default;
	virtual ~InputManager() = default;

public:
	// �L�[�{�[�h���擾
	static Keyboard* GetKeyboard() {
		return &InputManager::GetInstance().keyboard;
	}

	// �}�E�X���擾
	static Mouse* GetMouse() {
		return &InputManager::GetInstance().mouse;
	}

	// �Q�[���p�b�h���擾
	static GamePad* GetGamePad() {
		return &InputManager::GetInstance().gamePad;
	}

public:
	// �����ꂩ�̃L�[���p���I�ɉ�����Ă��邩�擾
	static bool GetAnyKey() noexcept {
		Keyboard* keyboard = InputManager::GetKeyboard();
		Mouse* mouse = InputManager::GetMouse();
		GamePad* gamePad = InputManager::GetGamePad();
		return (
			keyboard->GetAnyKey() || 
			mouse->GetAnyButton() || 
			gamePad->GetAnyButton());
	}

	// �����ꂩ�̃L�[�������ꂽ�u�Ԃ��ǂ����擾
	static bool GetAnyKeyDown() noexcept {
		Keyboard* keyboard = InputManager::GetKeyboard();
		Mouse* mouse = InputManager::GetMouse();
		GamePad* gamePad = InputManager::GetGamePad();
		return (
			keyboard->GetAnyKeyDown() ||
			mouse->GetAnyButtonDown() ||
			gamePad->GetAnyButtonDown());
	}

	// �����ꂩ�̃L�[�������ꂽ�u�Ԃ��ǂ����擾
	static bool GetAnyKeyUp() noexcept {
		Keyboard* keyboard = InputManager::GetKeyboard();
		Mouse* mouse = InputManager::GetMouse();
		GamePad* gamePad = InputManager::GetGamePad();
		return (
			keyboard->GetAnyKeyUp() ||
			mouse->GetAnyButtonUp() ||
			gamePad->GetAnyButtonUp());
	}

private:
	// �e�C���v�b�g�̏����ݒ�
	inline void StartUp(GLFWwindow* window) {
		keyboard.StartUp(window);
		mouse.StartUp(window);
		gamePad.StartUp(window);
	}

	// �e�C���v�b�g�̍X�V
	inline void UpdeteStates(float deltaSeconds) {
		keyboard.UpdeteStates(deltaSeconds);
		mouse.UpdeteStates(deltaSeconds);
		gamePad.UpdeteStates(deltaSeconds);
	}
};

#endif // INPUTMANAGER_H_INCLUDED