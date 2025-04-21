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
* インプットを管理する
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
	// キーボードを取得
	static Keyboard* GetKeyboard() {
		return &InputManager::GetInstance().keyboard;
	}

	// マウスを取得
	static Mouse* GetMouse() {
		return &InputManager::GetInstance().mouse;
	}

	// ゲームパッドを取得
	static GamePad* GetGamePad() {
		return &InputManager::GetInstance().gamePad;
	}

public:
	// いずれかのキーが継続的に押されているか取得
	static bool GetAnyKey() noexcept {
		Keyboard* keyboard = InputManager::GetKeyboard();
		Mouse* mouse = InputManager::GetMouse();
		GamePad* gamePad = InputManager::GetGamePad();
		return (
			keyboard->GetAnyKey() || 
			mouse->GetAnyButton() || 
			gamePad->GetAnyButton());
	}

	// いずれかのキーが押された瞬間かどうか取得
	static bool GetAnyKeyDown() noexcept {
		Keyboard* keyboard = InputManager::GetKeyboard();
		Mouse* mouse = InputManager::GetMouse();
		GamePad* gamePad = InputManager::GetGamePad();
		return (
			keyboard->GetAnyKeyDown() ||
			mouse->GetAnyButtonDown() ||
			gamePad->GetAnyButtonDown());
	}

	// いずれかのキーが離された瞬間かどうか取得
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
	// 各インプットの初期設定
	inline void StartUp(GLFWwindow* window) {
		keyboard.StartUp(window);
		mouse.StartUp(window);
		gamePad.StartUp(window);
	}

	// 各インプットの更新
	inline void UpdeteStates(float deltaSeconds) {
		keyboard.UpdeteStates(deltaSeconds);
		mouse.UpdeteStates(deltaSeconds);
		gamePad.UpdeteStates(deltaSeconds);
	}
};

#endif // INPUTMANAGER_H_INCLUDED