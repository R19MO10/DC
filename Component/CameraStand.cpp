/**
* @file CameraStand.cpp
*/
#include "CameraStand.h"

#include "../Utility/MyGameFunction.h"

#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"
#include "ThirdPersonView.h"


void CameraStand::Start()
{
	// 子オブジェクトの3人称視点コンポーネントを取得する
	if (const auto& child = GetOwner()->GetChild(0)) {
		child_ThirdPersonView = child->GetComponent<ThirdPersonView>();
	}

	if (!child_ThirdPersonView) {
		Debug::Log(GetFileName(__FILE__) + 
			"Cannot get child_ThirdPersonView");
	}
}

void CameraStand::Update(float deltaSeconds)
{
	GameObject* owner = GetOwner();

	vec3 targetPos = targetPosition;
	targetPos.y += heightOffset;
	
	// ターゲット座標にゆっくりと近づく
	const float dist = distance(owner->position, targetPos);
	const vec3 dir = normalize(targetPos - owner->position);

	vec3 addPos = dist * dir * deltaSeconds * trackSpeed;
	addPos.y *= 0.1f;	/* ジャンプをわかりやすくするためy軸方向の追跡は遅くする */
	owner->position += addPos;

	// 注視点からの距離が前フレームの距離と異なっていたら再設定を行う
	/* 毎フレーム lock を行うのを防ぐため */
	if (preTargetDistance != targetDistance) {
		if (isValid(child_ThirdPersonView)) {
			child_ThirdPersonView->targetDistance = targetDistance;
		}
		else {
			child_ThirdPersonView.reset();
			Debug::Log(GetFileName(__FILE__) + 
				"Cannot get child_ThirdPersonView");
		}

		preTargetDistance = targetDistance;
	}
}

void CameraStand::DrawImGui()
{
	ImGui::DragFloat3("TargetPosition", &targetPosition.x);
	ImGui::DragFloat("HeightOffset", &heightOffset);
	ImGui::DragFloat("TrackSpeed", &trackSpeed);
	ImGui::DragFloat("TargetDistance", &targetDistance);
}
