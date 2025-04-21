/**
* @file Keyboard.h
*/
#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED
#include "IInput.h"
#include <unordered_set>

/**
* キーボード入力状態管理クラス
*/
class Keyboard : public IInput
{
private:
	GLFWwindow* window = nullptr;	// ウィンドウポインタ

	std::unordered_set<int> keyStates;	// 現在フレームに押されたキー
	std::unordered_set<int> prevKeyStates;	// 前フレームに押されたキー

public:
	void StartUp(GLFWwindow* window) override;
	void UpdeteStates(float deltaSeconds) override;

	// キーが継続的に押されているか取得
	inline bool GetKey(int keyCode) const noexcept {
		// 現在フレームに押されている
		return keyStates.contains(keyCode);
	}
	// キーが押された瞬間かどうか取得
	inline bool GetKeyDown(int keyCode) const noexcept {
		// 現在フレームに押されていて、前フレームに押されていない
		return (keyStates.contains(keyCode) && !prevKeyStates.contains(keyCode));
	}
	// キーが離された瞬間かどうか取得
	inline bool GetKeyUp(int keyCode) const noexcept {
		// 現在フレームに押されておらず、前フレームに押されていた
		return (!keyStates.contains(keyCode) && prevKeyStates.contains(keyCode));
	}

	// いずれかのキーが継続的に押されているか取得
	inline bool GetAnyKey() const noexcept {
		return (keyStates.size() > 0);
	}
	//  いずれかのキーが押された瞬間かどうか取得
	inline bool GetAnyKeyDown() const noexcept {
		for (const auto& bCode : keyStates) {
			if (!prevKeyStates.contains(bCode)) {
				return true;
			}
		}
		return false;
	}
	//  いずれかのキーが離された瞬間かどうか取得
	inline bool GetAnyKeyUp() const noexcept {
		for (const auto& bCode : prevKeyStates) {
			if (!keyStates.contains(bCode)) {
				return true;
			}
		}
		return false;
	}
};

#endif // KEYBOARD_H_INCLUDED