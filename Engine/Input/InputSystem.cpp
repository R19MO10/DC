/**
* @file InputSystem.cpp
*/
#include "InputSystem.h"

#include "InputManager.h"
#include "InputActionMap.h"
#include <GLFW/glfw3.h>


/**
* 移動アクションの値を取得
* 
*    -1
* -1  0  1
*     1
*/
vec2 InputSystem::GetMoveActionValue()
{
	if (!InputSystem::GetInstance().isEnabled) {
		return vec2(0);
	}

	GamePad* gamePad = InputManager::GetGamePad();
	if (gamePad->GetConnectionState()) {
		return gamePad->GetLeftStick();
	}

	Keyboard* keyboard = InputManager::GetKeyboard();
	vec2 result = {
		static_cast<float>(keyboard->GetKey(GLFW_KEY_D) - keyboard->GetKey(GLFW_KEY_A)),
		static_cast<float>(keyboard->GetKey(GLFW_KEY_S) - keyboard->GetKey(GLFW_KEY_W))
	};

	return (result == 0.0f) ? vec2(0) : normalize(result);	/*0除算を回避するために判定を行う*/
}

/**
* 視点アクションの値を取得
*/
vec2 InputSystem::GetViewActionValue()
{
	if (!InputSystem::GetInstance().isEnabled) {
		return vec2(0);
	}

	GamePad* gamePad = InputManager::GetGamePad();
	if (gamePad->GetConnectionState()) {
		return gamePad->GetRightStick() * gamePad->stickSensitivity;
	}

	Mouse* mouse = InputManager::GetMouse();
	return mouse->GetDeltaMousePos() * mouse->mouseSensitivity;
}

/**
* アクションマップの有無効を取得する
*/
bool InputSystem::GetActionMapEnabled(const MyFName& mapName)
{
	// マップを取得
	if (const auto& map = 
		InputSystem::GetInstance().FindActionMap(mapName))
	{
		// 有無効を取得
		return map->isEnabled;
	}

	return false;
}

/**
* アクションマップの有無効を設定する
*/
void InputSystem::SetActionMapEnabled(
	const MyFName& mapName, bool enabled)
{
	// マップを取得
	if (const auto& map =
		InputSystem::GetInstance().FindActionMap(mapName))
	{
		// 有無効を設定
		map->isEnabled = enabled;
	}
}

/**
* アクションが終了した瞬間かを取得
*/
bool InputSystem::GetActionStateDown(
	const MyFName& mapName, const MyFName& actionName)
{
	auto& inst = InputSystem::GetInstance();
	if (!inst.isEnabled) {
		return false;
	}

	// マップを取得
	if (const auto& map = inst.FindActionMap(mapName))
	{
		// マップが有効か
		if (map->isEnabled) {
			return map->GetActionStateDown(actionName);
		}
	}

	return false;
}

/**
* アクションが開始した瞬間かを取得
*/
bool InputSystem::GetActionStateUp(
	const MyFName& mapName, const MyFName& actionName)
{
	auto& inst = InputSystem::GetInstance();
	if (!inst.isEnabled) {
		return false;
	}

	// マップを取得
	if (const auto& map = inst.FindActionMap(mapName))
	{
		// マップが有効か
		if (map->isEnabled) {
			return map->GetActionStateUp(actionName);
		}
	}

	return false;
}

/**
* アクションが実行中かを取得
*/
bool InputSystem::GetActionState(
	const MyFName& mapName, const MyFName& actionName)
{
	auto& inst = InputSystem::GetInstance();
	if (!inst.isEnabled) {
		return false;
	}

	// マップを取得
	if (const auto& map = inst.FindActionMap(mapName))
	{
		// マップが有効か
		if (map->isEnabled) {
			return map->GetActionState(actionName);
		}
	}

	return false;
}

// アクションマップを探す
InputActionMap* InputSystem::FindActionMap(
	const MyFName& mapName)
{
	// マップを探す
	for (auto& map : actionMaps) {
		if (map.mapName == mapName) {
			return &map;
		}
	}

	return nullptr;
}