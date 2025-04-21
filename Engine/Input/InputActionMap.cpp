/**
* @file InputActionMap.cpp
*/
#include "InputActionMap.h"

#include "../../Engine/DebugLog.h"
#include "InputSystem.h"
#include "InputManager.h"
#include <GLFW/glfw3.h>


// アクションが終了した瞬間かを取得
bool InputActionMap::GetActionStateDown(const MyFName& actionName)
{
	// 無効化されていれば処理しない
	if (!isEnabled) {
		return false;
	}

	const auto& actionData = actions.find(actionName);
	if (actionData == actions.end()) {
		// アクションが存在しない
		return false;
	}

	Keyboard* keyboard = InputManager::GetKeyboard();
	Mouse* mouse = InputManager::GetMouse();
	GamePad* gamePad = InputManager::GetGamePad();

	// 指定されたアクションに登録されているすべてのボタンを調べる
	for (const auto& action : (*actionData).second) {
		switch (action.kind)
		{
		case InputKind::Key:
			if (keyboard->GetKeyDown(action.code)) {
				return true;
			}
			break;

		case InputKind::MouseButton:
			if (mouse->GetMouseButtonDown(action.code)) {
				return true;
			}
			break;

		case InputKind::PadButton:
			if (gamePad->GetButtonDown(action.code)) {
				return true;
			}
			break;

		case InputKind::PadTrigger:
			switch (action.code)
			{
			case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER:
				if (gamePad->GetLeftTriggerDown()) {
					return true;
				}
				break;
			case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER:
				if (gamePad->GetRightTriggerDown()) {
					return true;
				}
				break;
			}
			break;

		case InputKind::MoveAction:
			LOG_WARNING("MoveActionの瞬間の取得はできません");
			break;

		case InputKind::ViewAction:
			LOG_WARNING("ViewActionの瞬間の取得はできません");
			break;
		}
	}

	return false;
}

// アクションが開始した瞬間かを取得
bool InputActionMap::GetActionStateUp(const MyFName& actionName)
{
	// 無効化されていれば処理しない
	if (!isEnabled) {
		return false;
	}

	const auto& actionData = actions.find(actionName);
	if (actionData == actions.end()) {
		// アクションが存在しない
		return false;
	}

	Keyboard* keyboard = InputManager::GetKeyboard();
	Mouse* mouse = InputManager::GetMouse();
	GamePad* gamePad = InputManager::GetGamePad();

	// 指定されたアクションに登録されているすべてのボタンを調べる
	for (const auto& action : (*actionData).second) {
		switch (action.kind)
		{
		case InputKind::Key:
			if (keyboard->GetKeyUp(action.code)) {
				return true;
			}
			break;

		case InputKind::MouseButton:
			if (mouse->GetMouseButtonUp(action.code)) {
				return true;
			}
			break;

		case InputKind::PadButton:
			if (gamePad->GetButtonUp(action.code)) {
				return true;
			}
			break;

		case InputKind::PadTrigger:
			switch (action.code)
			{
			case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER:
				if (gamePad->GetLeftTriggerUp()) {
					return true;
				}
				break;
			case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER:
				if (gamePad->GetRightTriggerUp()) {
					return true;
				}
				break;
			}
			break;

		case InputKind::MoveAction:
			LOG_WARNING("MoveActionの瞬間の取得はできません");
			break;

		case InputKind::ViewAction:
			LOG_WARNING("ViewActionの瞬間の取得はできません");
			break;
		}
	}

	return false;
}

// アクションが実行中かを取得
bool InputActionMap::GetActionState(const MyFName& actionName)
{
	// 無効化されていれば処理しない
	if (!isEnabled) {
		return false;
	}

	const auto& actionData = actions.find(actionName);
	if (actionData == actions.end()) {
		// アクションが存在しない
		return false;
	}

	Keyboard* keyboard = InputManager::GetKeyboard();
	Mouse* mouse = InputManager::GetMouse();
	GamePad* gamePad = InputManager::GetGamePad();

	// 指定されたアクションに登録されているすべてのボタンを調べる
	for (const auto& action : (*actionData).second) {
		switch (action.kind)
		{
		case InputKind::Key:
			if (keyboard->GetKey(action.code)) {
				return true;
			}
			break;

		case InputKind::MouseButton:
			if (mouse->GetMouseButton(action.code)) {
				return true;
			}
			break;

		case InputKind::PadButton:
			if (gamePad->GetButton(action.code)) {
				return true;
			}
			break;

		case InputKind::PadTrigger:
			switch (action.code)
			{
			case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER:
				if (gamePad->GetLeftTrigger() != 0.0f) {
					return true;
				}
				break;
			case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER:
				if (gamePad->GetRightTrigger() != 0.0f) {
					return true;
				}
				break;
			}
			break;

		case InputKind::MoveAction:
			if (InputSystem::GetMoveActionValue() != 0.0f) {
				return true;
			}
			break;

		case InputKind::ViewAction:
			if (InputSystem::GetViewActionValue() != 0.0f) {
				return true;
			}
			break;
		}
	}

	return false;
}
