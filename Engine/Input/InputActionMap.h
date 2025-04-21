/**
* @file InputActionMap.h
*/
#ifndef INPUTACTIONMAP_H_INCLUDED
#define INPUTACTIONMAP_H_INCLUDED
#include "../../Utility/MyFName.h"
#include <unordered_map>
#include <vector>

// インプットの種類
enum class InputKind
{
	Key,
	MouseButton,
	PadButton,
	PadTrigger,

	MoveAction,
	ViewAction,
};

// 登録情報
struct InputActionData {
	InputKind kind;	// インプットの集類
	int code = 0;	// インプットコード
};


/**
* インプットアクションマップ
*/
class InputActionMap
{
	friend class InputSystem;
private:
	MyFName mapName;

	using ActionList = 
		std::unordered_map<MyFName, std::vector<InputActionData>>;
	ActionList actions; // アクション管理リスト

public:
	bool isEnabled = true;	// マップの有効の有無

public:
	InputActionMap(const MyFName name)
		: mapName(name)
	{}
	virtual ~InputActionMap() = default;

public:
	// アクションを登録する
	inline void RegisterAction(
		const MyFName& actionName, std::vector<InputActionData>& data) 
	{
		// 同名アクションのデータを初期化する
		actions[actionName].clear();

		for (const auto& d : data) {
			actions[actionName].push_back(d);
		}
	}

public:
	// アクションが終了した瞬間かを取得
	bool GetActionStateDown(const MyFName& actionName);
	// アクションが開始した瞬間かを取得
	bool GetActionStateUp(const MyFName& actionName);
	// アクションが実行中かを取得
	bool GetActionState(const MyFName& actionName);
};

#endif // INPUTACTIONMAP_H_INCLUDED