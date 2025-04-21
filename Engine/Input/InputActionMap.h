/**
* @file InputActionMap.h
*/
#ifndef INPUTACTIONMAP_H_INCLUDED
#define INPUTACTIONMAP_H_INCLUDED
#include "../../Utility/MyFName.h"
#include <unordered_map>
#include <vector>

// �C���v�b�g�̎��
enum class InputKind
{
	Key,
	MouseButton,
	PadButton,
	PadTrigger,

	MoveAction,
	ViewAction,
};

// �o�^���
struct InputActionData {
	InputKind kind;	// �C���v�b�g�̏W��
	int code = 0;	// �C���v�b�g�R�[�h
};


/**
* �C���v�b�g�A�N�V�����}�b�v
*/
class InputActionMap
{
	friend class InputSystem;
private:
	MyFName mapName;

	using ActionList = 
		std::unordered_map<MyFName, std::vector<InputActionData>>;
	ActionList actions; // �A�N�V�����Ǘ����X�g

public:
	bool isEnabled = true;	// �}�b�v�̗L���̗L��

public:
	InputActionMap(const MyFName name)
		: mapName(name)
	{}
	virtual ~InputActionMap() = default;

public:
	// �A�N�V������o�^����
	inline void RegisterAction(
		const MyFName& actionName, std::vector<InputActionData>& data) 
	{
		// �����A�N�V�����̃f�[�^������������
		actions[actionName].clear();

		for (const auto& d : data) {
			actions[actionName].push_back(d);
		}
	}

public:
	// �A�N�V�������I�������u�Ԃ����擾
	bool GetActionStateDown(const MyFName& actionName);
	// �A�N�V�������J�n�����u�Ԃ����擾
	bool GetActionStateUp(const MyFName& actionName);
	// �A�N�V���������s�������擾
	bool GetActionState(const MyFName& actionName);
};

#endif // INPUTACTIONMAP_H_INCLUDED