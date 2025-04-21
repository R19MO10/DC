/**
* @file Mouse.cpp
*/
#include "Mouse.h"

#include "../../Engine/DebugLog.h"
#include <GLFW/glfw3.h>

void Mouse::StartUp(GLFWwindow* window)
{
	this->window = window;

	// マウス座標の計算
	double x, y;
	glfwGetCursorPos(window, &x, &y);	// スクリーン座標系のカーソル座標
	const vec2 pos = { static_cast<float>(x), static_cast<float>(y) };

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	const vec2 framebufferSize = { static_cast<float>(w), static_cast<float>(h) };

	const float aspectRatio = framebufferSize.x / framebufferSize.y;

	prevMousePos = mousePos = {
		(pos.x / framebufferSize.x * 2 - 1) * aspectRatio,
		(pos.y / framebufferSize.y * 2 - 1) * -1 };
}

void Mouse::UpdeteStates(float deltaSeconds)
{
	if (!window) {
		LOG_ERROR("[Mouse]：windowが設定されていません。StartUpが実行されていない可能性があります。");
		return;
	}

	// 前フレームのボタン状態を保存
	prevButtonStates.swap(buttonStates);
	buttonStates.clear();

	// 前フレームのマウス座標を保存
	prevMousePos = mousePos;

	// マウスのボタン状態を取得
	for (int bCode = GLFW_MOUSE_BUTTON_LEFT; bCode <= GLFW_MOUSE_BUTTON_MIDDLE; ++bCode) {
		if (glfwGetMouseButton(window, bCode) == GLFW_PRESS) {
			buttonStates.insert(bCode);
		}
	}

	// マウス座標の計算
	double x, y;
	glfwGetCursorPos(window, &x, &y);	// スクリーン座標系のカーソル座標
	const vec2 pos = { static_cast<float>(x), static_cast<float>(y) };

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	const vec2 framebufferSize = { static_cast<float>(w), static_cast<float>(h) };

	const float aspectRatio = framebufferSize.x / framebufferSize.y;

	/*
	UILayerの座標系に合わせるために、スクリーン座標系からカメラ座標系(Z=-1)に変換する
		カメラ座標系のマウス座標.x =
			((スクリーン座標系のマウス座標.x / 画面サイズ.x) * 2 - 1) * アスペクト比

		カメラ座標系のマウス座標.y =
			((スクリーン座標系のマウス座標.y / 画面サイズ.y) * 2 - 1) * -1

		「Y座標の式で-1を掛けているのは、スクリーン座標系とカメラ座標系ではY軸の方向が逆だから」
	*/
	mousePos = {
		(pos.x / framebufferSize.x * 2 - 1) * aspectRatio,
		(pos.y / framebufferSize.y * 2 - 1) * -1 };
}