/**
* @file CameraStand.h
*/
#ifndef CAMERASTAND_H_INCLUDED
#define CAMERASTAND_H_INCLUDED
#include "../Engine/Component.h"
#include "../Math/MyMath.h"

// ��s�錾
class GameObject;
using GameObject_WeakPtr = std::weak_ptr<GameObject>;
class ThirdPersonView;
using ThirdPersonViewPtr = std::shared_ptr<ThirdPersonView>;

/**
* �J�����̒����_
*/
class CameraStand : public Component
{
private:
	// �q�I�u�W�F�N�g�����L����3�l�̎��_�R���|�[�l���g�|�C���^
	ThirdPersonViewPtr child_ThirdPersonView;

	float preTargetDistance = 0;	// �O��̃t���[���̒����_����̋���

public:
	vec3 targetPosition;		// �����_�̈ʒu
	float heightOffset = 1;		// �����_��y���ɉ��Z����l
	float trackSpeed = 5;		// �����_��ǐՂ��鑬�x
	float targetDistance = 5;	//�����_����̋���

public:
	CameraStand()
		:Component("CameraStand")
	{}
	virtual ~CameraStand() = default;

	virtual void Start() override;
	virtual void Update(float deltaSeconds) override;
	virtual void DrawImGui() override;
};
using CameraStandPtr = std::shared_ptr<CameraStand>;

#endif // CAMERASTAND_H_INCLUDED
