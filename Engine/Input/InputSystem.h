/**
* @file InputSystem.h
*/
#ifndef INPUTSYSTEM_H_INCLUDED
#define INPUTSYSTEM_H_INCLUDED
#include "../../Utility/Singleton.h"
#include <vector>

// ��s�錾
class InputActionMap;

/**
* �A�N�V�����ʂ̃C���v�b�g���̎擾�㗝�N���X
*/
class InputSystem : public Singleton<InputSystem>
{
	friend class Engine;
private:
	bool isEnabled = true;
	std::vector<InputActionMap> actionMaps; // �A�N�V�����}�b�v�Ǘ����X�g

public:
	InputSystem() = default;
	virtual ~InputSystem() = default;

public:
	// �ړ��A�N�V�����̒l���擾
	static vec2 GetMoveActionValue();

	// ���_�A�N�V�����̒l���擾
	static vec2 GetViewActionValue();

public:
	// �A�N�V�����}�b�v�̗L�������擾����
	static bool GetActionMapEnabled(
		const MyFName& mapName);
	// �A�N�V�����}�b�v�̗L������ݒ肷��
	static void SetActionMapEnabled(
		const MyFName& mapName, bool enabled);

	// �A�N�V���������s�������擾
	static bool GetActionState(
		const MyFName& mapName, const MyFName& actionName);
	// �A�N�V�������I�������u�Ԃ����擾
	static bool GetActionStateDown(
		const MyFName& mapName, const MyFName& actionName);
	// �A�N�V�������J�n�����u�Ԃ����擾
	static bool GetActionStateUp(
		const MyFName& mapName, const MyFName& actionName);

private:
	// �A�N�V�����}�b�v��T��
	InputActionMap* FindActionMap(const MyFName& mapName);
};

#endif // INPUTSYSTEM_H_INCLUDED