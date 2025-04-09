#include "GamePad.h"

#include "../../Engine/DebugLog.h"
#include <GLFW/glfw3.h>
#undef APIENTRY
#include <Windows.h>
#undef max
#undef min
#include <Xinput.h>

#pragma comment(lib, "Xinput.lib")

struct Vibration {
	WORD leftMotorSpeed;  // 左モーター速度 (0～65535)
	WORD rightMotorSpeed; // 右モーター速度 (0～65535)
};


void GamePad::UpdeteStates(float deltaSeconds)
{
	isConnected = false;

	// 前フレームのボタン状態を保存
	prevButtonStates.swap(buttonStates);
	buttonStates.clear();

	leftStick = vec2(0);
	rightStick = vec2(0);

	preTrigger = trigger;
	trigger = vec2(0);

	// ゲームパットの状態を取得
	GLFWgamepadstate gamepadState;
	if (glfwGetGamepadState(0, &gamepadState)) {
		isConnected = true;

		// 各ボタンの状態
		for (int bCode = GLFW_GAMEPAD_BUTTON_A; bCode <= GLFW_GAMEPAD_BUTTON_LAST; ++bCode) {
			if (gamepadState.buttons[bCode] == GLFW_PRESS) {
				buttonStates.insert(bCode);
			}
		}

		// 左スティック
		leftStick.x = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
		leftStick.y = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
		for (int i = 0; i < 2; ++i) {
			// デッドゾーン以下なら値を0にする
			if (dot(leftStick[i]) <= dot(leftStickDeadZone)) {
				leftStick[i] = 0.0f;
			}
		}

		// 右スティック
		rightStick.x = gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
		rightStick.y = gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
		for (int i = 0; i < 2; ++i) {
			// デッドゾーン以下なら値を0にする
			if (dot(rightStick[i]) <= dot(rightStickDeadZone)) {
				rightStick[i] = 0.0f;
			}
		}

		// トリガー
		trigger.x = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
		trigger.y = gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
		// トリガー値を0～1に制限する
		trigger += 1;
		trigger *= 0.5f;
	}

	// バイブレーションのタイマーダウン
	if (vibaTimer_s > 0) {
		vibaTimer_s -= deltaSeconds;

		// タイマーが0になったらバイブレーションを止める
		if (vibaTimer_s <= 0) {
			XINPUT_STATE state;
			DWORD dwResult = XInputGetState(0, &state);
			if (dwResult == ERROR_SUCCESS) {
				Vibration vibration = { 0, 0 };

				// バイブレーションを停止
				XInputSetState(0, (XINPUT_VIBRATION*)&vibration);
				vibaPower = 0;
				vibaTimer_s = 0;
			}
		}
	}
}

/**
* バイブレーションを実行する
*
* @param power 振動の大きさ
* @param time  振動の継続時間
*/
void GamePad::Vibartion(float power, float time)
{
	if (power < 0) {
		return;
	}

	power = std::min(power, 100.0f);
	if (vibaPower > power) {
		// 現在実行中の振動よりも小さかったら処理しない
		return;
	}

	XINPUT_STATE state;
	DWORD dwResult = XInputGetState(0, &state);
	if (dwResult == ERROR_SUCCESS) {
		constexpr float VibartionPower = 65535.0f / 100.0f;
		WORD wpow = static_cast<WORD>(VibartionPower * power);
		Vibration vibration = { wpow, wpow };

		// バイブレーションを実行
		XInputSetState(0, (XINPUT_VIBRATION*)&vibration);
		vibaPower = power;
		vibaTimer_s = time;
	}
}
