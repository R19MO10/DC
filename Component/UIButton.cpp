/**
* @file UIButton.cpp
*/
#include "UIButton.h"

#include "../Engine/Engine.h"
#include "../Engine/Input/InputManager.h"

/**
* 更新イベント
*
* @param deltaSeconds 前回の更新からの経過時間(秒)
*/
void UIButton::Update(float deltaSeconds)
{
	// 基底クラスのメンバ関数を実行
	UILayout::Update(deltaSeconds);

	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	// ボタンの有効・無効判定
	if (!interactable) {
		std::copy_n(&normalColor.x, 3, &owner->color.x);
		owner->color.w = 0.75f; // 無効ぽさを出すため少し透明にする
		return;		
	}

	Mouse* mouse = InputManager::GetMouse();

	//マウス座標を取得
	const vec2 mousePos = mouse->GetMousePosition();

	if (abs(mousePos.x - owner->position.x) < owner->scale.x &&	/*abs() 絶対値をに変換*/
		abs(mousePos.y - owner->position.y) < owner->scale.y) {
		// カーソルがボタン領域内にある
		const bool buttonDown = mouse->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
		if (buttonDown) {
			// 左ボタンが押されている
			//色を押された時の色にする
			std::copy_n(&pressedColor.x, 3, &owner->color.x);	/*イテレータ範囲[first, first + n)をコピーする*/
		}
		else {
			// 左ボタンが離されている
			//色を離された時の色にする
			std::copy_n(&highlightedColor.x, 3, &owner->color.x);

			// ボタンクリック判定
			if (prevButtonDown) {
				// デリゲートを実行
				for (auto& e : onClick) {
					e(this);
				}
			}
		} // if buttonDown

		// マウスボタンの状態を更新
		prevButtonDown = buttonDown;
	}
	else {
		// カーソルがボタン領域外にある
		//色を通常に戻す
		std::copy_n(&normalColor.x, 3, &owner->color.x);
		prevButtonDown = false;
	}
}