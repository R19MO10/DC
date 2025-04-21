/**
* @file InputSystem.h
*/
#ifndef INPUTSYSTEM_H_INCLUDED
#define INPUTSYSTEM_H_INCLUDED
#include "../../Utility/Singleton.h"
#include <vector>

// 先行宣言
class InputActionMap;

/**
* アクション別のインプット情報の取得代理クラス
*/
class InputSystem : public Singleton<InputSystem>
{
	friend class Engine;
private:
	bool isEnabled = true;
	std::vector<InputActionMap> actionMaps; // アクションマップ管理リスト

public:
	InputSystem() = default;
	virtual ~InputSystem() = default;

public:
	// 移動アクションの値を取得
	static vec2 GetMoveActionValue();

	// 視点アクションの値を取得
	static vec2 GetViewActionValue();

public:
	// アクションマップの有無効を取得する
	static bool GetActionMapEnabled(
		const MyFName& mapName);
	// アクションマップの有無効を設定する
	static void SetActionMapEnabled(
		const MyFName& mapName, bool enabled);

	// アクションが実行中かを取得
	static bool GetActionState(
		const MyFName& mapName, const MyFName& actionName);
	// アクションが終了した瞬間かを取得
	static bool GetActionStateDown(
		const MyFName& mapName, const MyFName& actionName);
	// アクションが開始した瞬間かを取得
	static bool GetActionStateUp(
		const MyFName& mapName, const MyFName& actionName);

private:
	// アクションマップを探す
	InputActionMap* FindActionMap(const MyFName& mapName);
};

#endif // INPUTSYSTEM_H_INCLUDED