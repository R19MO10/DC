/**
* @file ThirdPersonView.cpp
*/
#include "ThirdPersonView.h"

#include <algorithm>
#include "../Engine/Input/InputSystem.h"

#include "../Engine/Engine.h"
#include "../Engine/DebugLog.h"


void ThirdPersonView::Start()
{
	GameObject* owner = GetOwner();
	// 若干の見下ろし視点から始める
	owner->rotation = { degrees_to_radians(-10), 0, 0};

	currentDistance = targetDistance;
}

void ThirdPersonView::EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat)
{
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	// 親オブジェクト(ターゲット)が設定されていなかったら処理しない
	if (!owner->GetParent()) {
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get ParentObject(targetObject)");
		return;
	}

	// 親オブジェクト(注視点)の座標を計算
	vec3 targetPos = vec3(owner->GetParent()->GetWorldTransformMatrix()[3]);

	// ゲームウィンドウが選択されいるか	
	if (engine->FocusWindow()) {
		// メインカメラが自身であるか
		if (owner == engine->GetMainCameraObject()) {
			// プレイヤーのアクションマップが有効であるか
			if (InputSystem::GetActionMapEnabled("Player")) {
				// 視点インプットデータを取得
				vec2 inputValue = InputSystem::GetViewActionValue();
				owner->rotation.x -= inputValue.y;
				owner->rotation.y -= inputValue.x;
			}
		}
	}

	// カメラの角度を制限（上下に逆さまになることを防ぐ）
	owner->rotation.x = std::clamp(owner->rotation.x, -1.2f, 0.9f);

	currentDistance += (targetDistance - currentDistance) * deltaSeconds * 2;

	// カメラの相対位置を極座標から直交座標に変換
	owner->position = {
		targetPos.x + currentDistance * std::cos(owner->rotation.x) * std::sin(owner->rotation.y),
		targetPos.y + currentDistance * std::sin(-owner->rotation.x),
		targetPos.z + currentDistance * std::cos(owner->rotation.x) * std::cos(owner->rotation.y),
	};

	// 壁や他のゲームオブジェクトにめり込まないようにする

	// ラムダ式で使用するために変数に代入
	MyFName playerTag("Player");

	// カメラがめり込んでいるコライダーを探す
	ColliderList hitColliders =
		engine->InsidePoint_GameObjectAll(owner->position,
			[&playerTag](const ColliderPtr& collider) {
				return
					!collider->isTrigger &&
					collider->type != CollisionType::BlockingVolume &&
					collider->GetOwner()->GetTag() != playerTag;
			});

	// カメラが何かにめり込んでいるか
	if (!hitColliders.empty()) {
		// ターゲットからカメラへ向けての光線を作成
		const vec3 direc = normalize(owner->position - targetPos);
		const Ray ray = { targetPos, direc };

		// めり込んでいるコライダーとターゲットからカメラへ向けての光線との交差座標を取得する
		RayHitInfo hitInfo;
		Raycast(ray, hitColliders, hitInfo,
			[](const ColliderPtr& collider, float distance) {
				return true; });

		owner->position = hitInfo.point;
		currentDistance = hitInfo.distance;
	}

	// 最後に自身のワールド座標変換行列を上書きする
	ownerWorldTransMat = 
		TransformMatrix(owner->position, owner->rotation, owner->scale);
}

void ThirdPersonView::DrawImGui()
{
	ImGui::DragFloat("Target Distance", &targetDistance);
	ImGui::DragFloat("Current Distance", &currentDistance);
}
