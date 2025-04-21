/**
* @file UILayout.cpp
*/
#include "UILayout.h"

#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"

void UILayout::EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat)
{
	// 常にカメラの方を向くようにゲームオブジェクトの向きを調整(ビルボードと同じ)
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	if (GameObject* cameraObj = engine->GetMainCameraObject()) {
		// 常にカメラの正面に位置するようにゲームオブジェクトの座標を調整
		// Y座標の-1～+1が、カメラに写る範囲の上端と下端になるように、Z座標を調整
		vec3 pos = vec3(ownerWorldTransMat[3]);
		pos.z = -engine->GetMainCamera_CameraComponent()->fovScale.y;	// 垂直

		// カメラとの相対座標を求める
		pos = vec3(cameraObj->GetWorldTransformMatrix() * vec4(pos, 1));

		vec3 scale = owner->scale;
		// 親が設定されていた場合は座標変換行列から拡大率を抽出
		/* 除算を減らすために一応確認している */
		if (owner->GetParent()) {
			scale = ExtractScale(ownerWorldTransMat);
		}

		ownerWorldTransMat =
			TransformMatrix(pos, mat3(cameraObj->GetWorldTransformMatrix()), scale);
	}
}

void UILayout::DrawImGui()
{
	ImGui::DragInt("Priority", &priority);
}
