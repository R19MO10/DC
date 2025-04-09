/**
* @file Keyboard.cpp
*/
#include "Keyborad.h"

#include "../../Engine/DebugLog.h"
#include <GLFW/glfw3.h>

void Keyboard::StartUp(GLFWwindow* window) 
{
	this->window = window;
}

void Keyboard::UpdeteStates(float deltaSeconds)
{
	if (!window) {
		LOG_ERROR("windowが設定されていません。StartUpが実行されていない可能性があります。");
		return;
	}

	// 前フレームに押されたボタンを格納
	prevKeyStates.swap(keyStates);
	keyStates.clear();

	// PrintableKeyの状態を取得
	for (int keyCode = GLFW_KEY_SPACE; keyCode <= GLFW_KEY_GRAVE_ACCENT; ++keyCode) {
		if (glfwGetKey(window, keyCode) == GLFW_PRESS) {
			keyStates.insert(keyCode);
		}
	}

	// FunctionKeyの状態を取得
	for (int keyCode = GLFW_KEY_ESCAPE; keyCode <= GLFW_KEY_MENU; ++keyCode) {
		if (glfwGetKey(window, keyCode) == GLFW_PRESS) {
			keyStates.insert(keyCode);
		}
	}

}