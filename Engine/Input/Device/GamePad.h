/**
* @file GamePad.h
*/
#ifndef GAMEPAD_H_INCLUDED
#define GAMEPAD_H_INCLUDED
#include"IInput.h"
#include "../../Math/MyMath.h"
#include <unordered_set>

/**
* マウス入力状態管理
*/
class GamePad : public IInput
{
private:
	bool isConnected = false;

	std::unordered_set<int> buttonStates;		// 現フレームの押されたボタン
	std::unordered_set<int> prevButtonStates;	// 前フレームの押されたボタン

	vec2 leftStick = vec2(0);
	vec2 rightStick = vec2(0);

	vec2 trigger = vec2(0);
	vec2 preTrigger = vec2(0);

	float vibaPower = 0;	// バイブレーションパワー
	float vibaTimer_s = 0;	// バイブレーション持続時間

public:
	float leftStickDeadZone = 0.1f;		// スティックデッドゾーン
	float rightStickDeadZone = 0.1f;

	float stickSensitivity = 0.1f;	// スティック感度

public:
	void StartUp(GLFWwindow* window) override {}
	void UpdeteStates(float deltaSeconds) override;

	// ゲームパットが接続されているか取得
	inline bool GetConnectionState() { return isConnected; }

	// ボタンが継続的に押されているか取得
	inline bool GetButton(int buttonCode) const noexcept {
		// 現在フレームに押されている
		return buttonStates.contains(buttonCode);
	}
	// ボタンが押された瞬間かどうか取得
	inline bool GetButtonDown(int buttonCode) const noexcept {
		// 現在フレームに押されていて、前フレームに押されていない
		return (buttonStates.contains(buttonCode) && !prevButtonStates.contains(buttonCode));
	}
	// ボタンが離された瞬間かどうか取得
	inline bool GetButtonUp(int buttonCode) const noexcept {
		// 現在フレームに押されておらず、前フレームに押されていた
		return (!buttonStates.contains(buttonCode) && prevButtonStates.contains(buttonCode));
	}

	// いずれかのボタンが継続的に押されているか取得
	inline bool GetAnyButton() const noexcept {
		return (buttonStates.size() > 0);
	}
	// いずれかのボタンが押された瞬間か取得
	inline bool GetAnyButtonDown() const noexcept {
		for (const auto& bCode : buttonStates) {
			if (!prevButtonStates.contains(bCode)) {
				return true;
			}
		}
		return false;
	}
	// いずれかのボタンが離された瞬間か取得
	inline bool GetAnyButtonUp() const noexcept {
		for (const auto& bCode : prevButtonStates) {
			if (!buttonStates.contains(bCode)) {
				return true;
			}
		}
		return false;
	}

	// 左スティックの値を取得
	inline vec2 GetLeftStick() const noexcept { return leftStick; }
	// 右スティックの値を取得
	inline vec2 GetRightStick() const noexcept { return rightStick; }

	// 左トリガーの値を取得
	inline float GetLeftTrigger() const noexcept { return trigger.x; }
	// 左トリガーが押されて瞬間かどうか取得
	inline bool GetLeftTriggerDown() const noexcept { 
		return (preTrigger.x == 0.0f && trigger.x > 0);
	}
	// 左トリガーが離された瞬間かどうか取得
	inline bool GetLeftTriggerUp() const noexcept { 
		return (preTrigger.x != 0.0f && trigger.x == 0);
	}

	// 右トリガーの値を取得
	inline float GetRightTrigger() const noexcept { return trigger.y; }
	// 右トリガーが押されて瞬間かどうか取得
	inline bool GetRightTriggerDown() const noexcept {
		return (preTrigger.y == 0.0f && trigger.y > 0);
	}
	// 右トリガーが離された瞬間かどうか取得
	inline bool GetRightTriggerUp() const noexcept {
		return (preTrigger.y != 0.0f && trigger.y == 0);
	}

	/**
	* バイブレーションを実行する
	*
	* @param power 振動の大きさ
	* @param time  振動の継続時間
	*/
	void Vibartion(float power, float time = 0.1f);
};

#endif // MOUSE_H_INCLUDED