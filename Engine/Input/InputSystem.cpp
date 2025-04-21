/**
* @file InputSystem.cpp
*/
#include "InputSystem.h"

#include "InputManager.h"
#include "InputActionMap.h"
#include <GLFW/glfw3.h>


/**
* �ړ��A�N�V�����̒l���擾
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

	return (result == 0.0f) ? vec2(0) : normalize(result);	/*0���Z��������邽�߂ɔ�����s��*/
}

/**
* ���_�A�N�V�����̒l���擾
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
* �A�N�V�����}�b�v�̗L�������擾����
*/
bool InputSystem::GetActionMapEnabled(const MyFName& mapName)
{
	// �}�b�v���擾
	if (const auto& map = 
		InputSystem::GetInstance().FindActionMap(mapName))
	{
		// �L�������擾
		return map->isEnabled;
	}

	return false;
}

/**
* �A�N�V�����}�b�v�̗L������ݒ肷��
*/
void InputSystem::SetActionMapEnabled(
	const MyFName& mapName, bool enabled)
{
	// �}�b�v���擾
	if (const auto& map =
		InputSystem::GetInstance().FindActionMap(mapName))
	{
		// �L������ݒ�
		map->isEnabled = enabled;
	}
}

/**
* �A�N�V�������I�������u�Ԃ����擾
*/
bool InputSystem::GetActionStateDown(
	const MyFName& mapName, const MyFName& actionName)
{
	auto& inst = InputSystem::GetInstance();
	if (!inst.isEnabled) {
		return false;
	}

	// �}�b�v���擾
	if (const auto& map = inst.FindActionMap(mapName))
	{
		// �}�b�v���L����
		if (map->isEnabled) {
			return map->GetActionStateDown(actionName);
		}
	}

	return false;
}

/**
* �A�N�V�������J�n�����u�Ԃ����擾
*/
bool InputSystem::GetActionStateUp(
	const MyFName& mapName, const MyFName& actionName)
{
	auto& inst = InputSystem::GetInstance();
	if (!inst.isEnabled) {
		return false;
	}

	// �}�b�v���擾
	if (const auto& map = inst.FindActionMap(mapName))
	{
		// �}�b�v���L����
		if (map->isEnabled) {
			return map->GetActionStateUp(actionName);
		}
	}

	return false;
}

/**
* �A�N�V���������s�������擾
*/
bool InputSystem::GetActionState(
	const MyFName& mapName, const MyFName& actionName)
{
	auto& inst = InputSystem::GetInstance();
	if (!inst.isEnabled) {
		return false;
	}

	// �}�b�v���擾
	if (const auto& map = inst.FindActionMap(mapName))
	{
		// �}�b�v���L����
		if (map->isEnabled) {
			return map->GetActionState(actionName);
		}
	}

	return false;
}

// �A�N�V�����}�b�v��T��
InputActionMap* InputSystem::FindActionMap(
	const MyFName& mapName)
{
	// �}�b�v��T��
	for (auto& map : actionMaps) {
		if (map.mapName == mapName) {
			return &map;
		}
	}

	return nullptr;
}