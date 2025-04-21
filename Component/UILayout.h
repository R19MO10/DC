/**
* @file UILayout.h
*/
#ifndef UILAYOUT_H_INCLUDED
#define UILAYOUT_H_INCLUDED
#include "../Engine/Component.h"
#include "../Math/MyMath.h"

/**
* UI����ʋ�ԂŔz�u���邽�߂̃R���|�[�l���g
*/
class UILayout : public Component
{
public:
	int priority = 0;	// �\���D��x(������Ύ�O�ɕ\�������)

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