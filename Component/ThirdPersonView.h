/**
* @file ThirdPersonView.h
*/
#ifndef THIRDPERSONVIEW_H_INCLUDED
#define THIRDPERSONVIEW_H_INCLUDED
#include "../Engine/Component.h"

/**
* 三人称視点制御クラス(親オブジェクトを注視点とする)
*/
class ThirdPersonView : public Component 
{
public:
	float targetDistance = 4;	// 注目点からの距離
	float currentDistance = 0;	// 現在の注目点からの距離

public:
	ThirdPersonView() 
		:Component("ThirdPersonView")
	{};
	virtual ~ThirdPersonView() = default;

public:
	virtual void Start() override;
	virtual void EndUpdate(
		float deltaSeconds, mat4& ownerWorldTransMat) override;
	virtual void DrawImGui() override;
};
using ThirdPersonViewPtr = std::shared_ptr<ThirdPersonView>;

#endif // THIRDPERSONVIEW_H_INCLUDED