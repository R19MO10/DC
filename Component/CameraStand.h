/**
* @file CameraStand.h
*/
#ifndef CAMERASTAND_H_INCLUDED
#define CAMERASTAND_H_INCLUDED
#include "../Engine/Component.h"
#include "../Math/MyMath.h"

// 先行宣言
class GameObject;
using GameObject_WeakPtr = std::weak_ptr<GameObject>;
class ThirdPersonView;
using ThirdPersonViewPtr = std::shared_ptr<ThirdPersonView>;

/**
* カメラの注視点
*/
class CameraStand : public Component
{
private:
	// 子オブジェクトが所有する3人称視点コンポーネントポインタ
	ThirdPersonViewPtr child_ThirdPersonView;

	float preTargetDistance = 0;	// 前回のフレームの注視点からの距離

public:
	vec3 targetPosition;		// 注視点の位置
	float heightOffset = 1;		// 注視点のy軸に加算する値
	float trackSpeed = 5;		// 注視点を追跡する速度
	float targetDistance = 5;	//注視点からの距離

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
