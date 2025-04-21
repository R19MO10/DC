/**
* @file Camera.cpp
*/
#include "Camera.h"

#include "../Utility/Random.h"

#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"
#include "../Engine/Input/InputManager.h"


void Camera::EndUpdate(
	float deltaSeconds, mat4& ownerWorldTransMat)
{
	// 遠近法パラメータA, Bを計算
	// （深度値の計算結果が-1～+1になるようなパラメータA, B）
	/* パースペクティブ行列の逆の計算を行い、ビュー座標系に変換される */
	A = -2 * far * near / (far - near);
	B = (far + near) / (far - near);

	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	// 視野角による拡大率を計算
	fovScale.y =	// 垂直
		1 / tan(fovY / 2);			
	fovScale.x =	//水平
		fovScale.y * (1 / engine->GetAspectRatio());	

	vec3 position;
	mat3 rotationMat;
	vec3 scale;
	Decompose(
		ownerWorldTransMat, position, rotationMat, scale);

	// シェイク処理
	if (isShake) {
		shake_elapsedTime_s += deltaSeconds * shakeSpeed;

		// シェイク座標を計算
		/* 縦振動のみを起こしている */
		vec3 sPos = {
			0, std::sin(shake_elapsedTime_s) * currentShakePower, 0
		};

		// 振動を適応
		position += rotationMat * sPos;

		// 振動の強さをだんだん下げていく
		currentShakePower -= deltaSeconds * maxShakePower * 2;

		// 振動の強さがなくなったら振動を終わる
		if (currentShakePower < 0) {
			shake_elapsedTime_s = 0;
			isShake = false;
		}
	}

	// ビュー行列を作成
	const vec3 eye = position;
	const vec3 target =
		position * rotationMat[2];
	matView = LookAt(eye, target, vec3(0, 1, 0));

	// ビュープロジェクション行列を作成
	matProj =
		Perspective(fovY, engine->GetAspectRatio(), near, far);

	// 最後に振動を適応ワールド座標変換行列を上書きする
	ownerWorldTransMat =
		TransformMatrix(position, rotationMat, scale);
}

void Camera::DrawImGui()
{
	ImGui::DragFloat("Fov Y", &fovY);
	ImGui::Spacing();
	ImGui::DragFloat("Near", &near);
	ImGui::DragFloat("Far", &far);
	ImGui::Spacing();
	ImGui::DragFloat("A", &A);
	ImGui::DragFloat("B", &B);
	ImGui::Spacing();

	// カメラシェイクを任意実行
	static float sPwr = 20;
	ImGui::DragFloat("ShakeSpeed", &shakeSpeed);
	ImGui::DragFloat("ShakePower", &sPwr);
	if (ImGui::Button("CameraShake!!")) {
		isShake = true;
		currentShakePower = maxShakePower = sPwr * 0.001f;
	}
}