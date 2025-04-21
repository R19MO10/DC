/**
* @file UIButton.h
*/
#ifndef UIBUTTON_H_INCLUDED
#define UIBUTTON_H_INCLUDED
#include "UILayout.h"
#include <functional>

/**
* UI用のボタン制御コンポーネント
*/
class UIButton : public UILayout
{
public:
	bool interactable = true; // マウスに反応を、true=する false=しない
	vec3 normalColor = { 1.0f, 1.0f, 1.0f };		// 通常色
	vec3 highlightedColor = { 1.3f, 1.3f, 1.3f };	// 選択色
	vec3 pressedColor = { 0.7f, 0.7f, 0.7f };		// 押下色

	// デリゲート（
	/* std::function<戻り型(引数リスト)>;　
		function型には関数、関数オブジェクト、ラムダ式を代入して変数として扱うことができる）*/
	using ClickEvent = std::function<void(UIButton*)>;
	std::vector<ClickEvent> onClick;

private:
	bool prevButtonDown = false; // 前回更新時のマウス左ボタン操作状態

public:
	UIButton() 
		:UILayout("UIButton")
	{};
	virtual ~UIButton() = default;

public:
	virtual void Update(float deltaSeconds) override;
};
using UIButtonPtr = std::shared_ptr<UIButton>;


#endif // UIBUTTON_H_INCLUDED