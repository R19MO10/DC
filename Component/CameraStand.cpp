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
	// �q�I�u�W�F�N�g��3�l�̎��_�R���|�[�l���g���擾����
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
	
	// �^�[�Q�b�g���W�ɂ������Ƌ߂Â�
	const float dist = distance(owner->position, targetPos);
	const vec3 dir = normalize(targetPos - owner->position);

	vec3 addPos = dist * dir * deltaSeconds * trackSpeed;
	addPos.y *= 0.1f;	/* �W�����v���킩��₷�����邽��y�������̒ǐՂ͒x������ */
	owner->position += addPos;

	// �����_����̋������O�t���[���̋����ƈقȂ��Ă�����Đݒ���s��
	/* ���t���[�� lock ���s���̂�h������ */
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
