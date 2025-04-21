/**
* @file UILayout.cpp
*/
#include "UILayout.h"

#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"

void UILayout::EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat)
{
	// ��ɃJ�����̕��������悤�ɃQ�[���I�u�W�F�N�g�̌����𒲐�(�r���{�[�h�Ɠ���)
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	if (GameObject* cameraObj = engine->GetMainCameraObject()) {
		// ��ɃJ�����̐��ʂɈʒu����悤�ɃQ�[���I�u�W�F�N�g�̍��W�𒲐�
		// Y���W��-1�`+1���A�J�����Ɏʂ�͈͂̏�[�Ɖ��[�ɂȂ�悤�ɁAZ���W�𒲐�
		vec3 pos = vec3(ownerWorldTransMat[3]);
		pos.z = -engine->GetMainCamera_CameraComponent()->fovScale.y;	// ����

		// �J�����Ƃ̑��΍��W�����߂�
		pos = vec3(cameraObj->GetWorldTransformMatrix() * vec4(pos, 1));

		vec3 scale = owner->scale;
		// �e���ݒ肳��Ă����ꍇ�͍��W�ϊ��s�񂩂�g�嗦�𒊏o
		/* ���Z�����炷���߂Ɉꉞ�m�F���Ă��� */
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
