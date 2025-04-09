#pragma once

#if 0
#include <string>
#include <memory>
#include "../Math/MyMath.h"

struct GLFWwindow;
class GameObject;
using GameObject_WeakPtr = std::weak_ptr<GameObject>;

class DebugWindow
{
private:
	GLFWwindow* window = nullptr;				// ウィンドウオブジェクト
	const std::string title = "Debug ImGui";	// ウィンドウタイトル

	GameObject_WeakPtr selectingGameObjct;	// 選択中のゲームオブジェクト

	bool showCollision = false;	// コリジョンを可視化
	vec4 collisionColor[4] = {	// コリジョン表示用カラー
		{ 1.0f, 0.0f, 0.2f, 0.5f },	// 通常
		{ 0.1f, 0.1f, 0.1f, 0.5f },	// 非アクティブ
		{ 1.0f, 1.0f, 0.0f, 0.5f },	// トリガー
		{ 0.0f, 1.0f, 0.5f, 0.5f },	// スタティック
	};

public:
	DebugWindow();
	virtual ~DebugWindow();

public:
	RenderImGui()
};

#endif