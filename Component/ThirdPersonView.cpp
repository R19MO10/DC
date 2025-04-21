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
	// �኱�̌����낵���_����n�߂�
	owner->rotation = { degrees_to_radians(-10), 0, 0};

	currentDistance = targetDistance;
}

void ThirdPersonView::EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat)
{
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	// �e�I�u�W�F�N�g(�^�[�Q�b�g)���ݒ肳��Ă��Ȃ������珈�����Ȃ�
	if (!owner->GetParent()) {
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get ParentObject(targetObject)");
		return;
	}

	// �e�I�u�W�F�N�g(�����_)�̍��W���v�Z
	vec3 targetPos = vec3(owner->GetParent()->GetWorldTransformMatrix()[3]);

	// �Q�[���E�B���h�E���I�����ꂢ�邩	
	if (engine->FocusWindow()) {
		// ���C���J���������g�ł��邩
		if (owner == engine->GetMainCameraObject()) {
			// �v���C���[�̃A�N�V�����}�b�v���L���ł��邩
			if (InputSystem::GetActionMapEnabled("Player")) {
				// ���_�C���v�b�g�f�[�^���擾
				vec2 inputValue = InputSystem::GetViewActionValue();
				owner->rotation.x -= inputValue.y;
				owner->rotation.y -= inputValue.x;
			}
		}
	}

	// �J�����̊p�x�𐧌��i�㉺�ɋt���܂ɂȂ邱�Ƃ�h���j
	owner->rotation.x = std::clamp(owner->rotation.x, -1.2f, 0.9f);

	currentDistance += (targetDistance - currentDistance) * deltaSeconds * 2;

	// �J�����̑��Έʒu���ɍ��W���璼�����W�ɕϊ�
	owner->position = {
		targetPos.x + currentDistance * std::cos(owner->rotation.x) * std::sin(owner->rotation.y),
		targetPos.y + currentDistance * std::sin(-owner->rotation.x),
		targetPos.z + currentDistance * std::cos(owner->rotation.x) * std::cos(owner->rotation.y),
	};

	// �ǂ⑼�̃Q�[���I�u�W�F�N�g�ɂ߂荞�܂Ȃ��悤�ɂ���

	// �����_���Ŏg�p���邽�߂ɕϐ��ɑ��
	MyFName playerTag("Player");

	// �J�������߂荞��ł���R���C�_�[��T��
	ColliderList hitColliders =
		engine->InsidePoint_GameObjectAll(owner->position,
			[&playerTag](const ColliderPtr& collider) {
				return
					!collider->isTrigger &&
					collider->type != CollisionType::BlockingVolume &&
					collider->GetOwner()->GetTag() != playerTag;
			});

	// �J�����������ɂ߂荞��ł��邩
	if (!hitColliders.empty()) {
		// �^�[�Q�b�g����J�����֌����Ă̌������쐬
		const vec3 direc = normalize(owner->position - targetPos);
		const Ray ray = { targetPos, direc };

		// �߂荞��ł���R���C�_�[�ƃ^�[�Q�b�g����J�����֌����Ă̌����Ƃ̌������W���擾����
		RayHitInfo hitInfo;
		Raycast(ray, hitColliders, hitInfo,
			[](const ColliderPtr& collider, float distance) {
				return true; });

		owner->position = hitInfo.point;
		currentDistance = hitInfo.distance;
	}

	// �Ō�Ɏ��g�̃��[���h���W�ϊ��s����㏑������
	ownerWorldTransMat = 
		TransformMatrix(owner->position, owner->rotation, owner->scale);
}

void ThirdPersonView::DrawImGui()
{
	ImGui::DragFloat("Target Distance", &targetDistance);
	ImGui::DragFloat("Current Distance", &currentDistance);
}
