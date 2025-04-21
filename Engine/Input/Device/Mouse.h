/**
* @file Mouse.h
*/
#ifndef MOUSE_H_INCLUDED
#define MOUSE_H_INCLUDED
#include "IInput.h"
#include "../../Math/MyMath.h"
#include <unordered_set>

/**
* マウス入力状態管理
*/
class Mouse : public IInput
{
private:
	GLFWwindow* window = nullptr;	// ウィンドウポインタ

	std::unordered_set<int> buttonStates;		// 現フレームの押されたボタン
	std::unordered_set<int> prevButtonStates;	// 前フレームの押されたボタン

	vec2 mousePos = vec2(0);		// 現フレームのマウスの座標
	vec2 prevMousePos = vec2(0);	// 前フレームのマウス座標

public:
	float mouseSensitivity = 1;	// マウス感度

public:
	void StartUp(GLFWwindow* window) override;
	void UpdeteStates(float deltaSeconds) override;

	// ボタンが継続的に押されているか取得
	inline bool GetMouseButton(int buttonCode) const noexcept {
		// 現在フレームに押されている
		return buttonStates.contains(buttonCode);
	}
	// ボタンが押された瞬間かどうか
	inline bool GetMouseButtonDown(int buttonCode) const noexcept {
		// 現在フレームに押されていて、前フレームに押されていない取得
		return (buttonStates.contains(buttonCode) && !prevButtonStates.contains(buttonCode));
	}
	// ボタンが離された瞬間かどうか
	inline bool GetMouseButtonUp(int buttonCode) const noexcept {
		// 現在フレームに押されておらず、前フレームに押されていた取得
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

	// マウスカーソルの座標(カメラ座標系(Z=-1))を取得
	inline vec2 GetMousePosition() const noexcept { return mousePos; }
	// マウスの相対移動量を取得
	inline vec2 GetDeltaMousePos() const noexcept {
		return {
			mousePos.x - prevMousePos.x,
			prevMousePos.y - mousePos.y,
		};
	}
};

#endif // MOUSE_H_INCLUDED