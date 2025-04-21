/**
* @file ThirdPersonView.h
*/
#ifndef THIRDPERSONVIEW_H_INCLUDED
#define THIRDPERSONVIEW_H_INCLUDED
#include "../Engine/Component.h"

/**
* �O�l�̎��_����N���X(�e�I�u�W�F�N�g�𒍎��_�Ƃ���)
*/
class ThirdPersonView : public Component 
{
public:
	float targetDistance = 4;	// ���ړ_����̋���
	float currentDistance = 0;	// ���݂̒��ړ_����̋���

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