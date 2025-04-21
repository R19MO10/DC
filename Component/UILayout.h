/**
* @file UILayout.h
*/
#ifndef UILAYOUT_H_INCLUDED
#define UILAYOUT_H_INCLUDED
#include "../Engine/Component.h"
#include "../Math/MyMath.h"

/**
* UIを画面空間で配置するためのコンポーネント
*/
class UILayout : public Component
{
public:
	int priority = 0;	// 表示優先度(高ければ手前に表示される)

public:
	UILayout(const char* compName = "UILayout")
		:Component(compName)
	{};
	virtual ~UILayout() = default;

public:
	virtual void EndUpdate(
		float deltaSeconds, mat4& ownerWorldTransMat) override;
	virtual void DrawImGui() override;
};
using UILayoutPtr = std::shared_ptr<UILayout>;

#endif // UILAYOUT_H_INCLUDED