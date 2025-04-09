/**
* @file ColliderDetection.cpp
*/
#include "ColliderDetection.h"

#include "../GameObject.h"
#include "../../Component/Collider.h"
#include "../../Component/AabbCollider.h"
#include "../../Component/BoxCollider.h"
#include "../../Component/SphereCollider.h"
#include "../../Component/Rigidbody.h"


void WorldCollider::AddPosition(const vec3& v)
{
	origin->GetOwner()->position += v;
	world->AddPosition(v);
}

namespace {
	/**
	* �ђʃx�N�g�����Q�[���I�u�W�F�N�g�ɔ��f����(�R���C�_�[���d�Ȃ�Ȃ��悤�ɍ��W�𒲐�)
	*/
	void ApplyPenetration(WorldColliderList* worldColliders,
		GameObject* gameObject, const vec3& penetration)
	{
		// ���W�b�h�{�f�B������ΐڒn������s��
		if (auto rigid = gameObject->GetRigidbody()) {
			// �Փ˃x�N�g���������ɋ߂��ꍇ�ɁA���ɐG�ꂽ�Ƃ݂Ȃ�
			static const float cosGround = cos(degrees_to_radians(30)); // ���Ƃ݂Ȃ��p�x

			// �Փˊp�x��90�x���傫���ꍇ��
			// ���炩�ɕ�(�V��)�Ȃ̂ŏ��O����
			if (penetration.y > 0) {

				// �Ώۂ��P�ʐ����x�N�g���ł��邱�Ƃ𗘗p���āA
				// ���ςɂ��p�x�̔�r��P����
				const float d = length(penetration);

				if (penetration.y >= d * cosGround) {
					rigid->isGrounded = true; // �ڒn����
				}
			} // if penetration.y
		}

		// �Q�[���I�u�W�F�N�g���ړ�
		gameObject->position += penetration;

		// �S�Ẵ��[���h�R���C�_�[���ړ�
		for (auto& e : *worldColliders) {
			e.world->AddPosition(penetration);
		}
	}

	/**
	* �R���C�_�[�^�ɑΉ�����Փ˔���֐����Ăяo��
	*/
	template<typename ColA, typename ColB>
	bool Func(const ColliderPtr& a, const ColliderPtr& b, ContactPoint& cp)
	{
		return Intersect(static_cast<ColA&>(*a).GetShape(),
			static_cast<ColB&>(*b).GetShape(), cp);
	}

	template<typename ColA, typename ColB>
	bool NotFunc(const ColliderPtr& a, const ColliderPtr& b, ContactPoint& cp) {
		return false;
	}

	/**
	* �R���C�_�[�P�ʂ̏Փ˔���
	*
	* @param a ����Ώۂ̃��[���h�R���C�_�[�z�񂻂�1
	* @param b ����Ώۂ̃��[���h�R���C�_�[�z�񂻂�2
	*/
	void HandleWorldColliderCollision(
		WorldColliderList* a, WorldColliderList* b)
	{
		// �֐��|�C���^�^���`
		using FuncType = bool(*)(const ColliderPtr&, const ColliderPtr&, ContactPoint&);

		// �g�ݍ��킹�ɑΉ������������֐���I�Ԃ��߂̔z��
		static const FuncType funcList[3][3] = {
			{	// AABB
				Func<AabbCollider, AabbCollider>,		Func<AabbCollider, SphereCollider>,		Func<AabbCollider, BoxCollider>
			},
			{	// ����
				Func<SphereCollider, AabbCollider>,		Func<SphereCollider, SphereCollider>,	Func<SphereCollider, BoxCollider>
			},
			{	// Box
				Func<BoxCollider, AabbCollider>,		Func<BoxCollider, SphereCollider>,		Func<BoxCollider, BoxCollider>,
			},
		};

		// �R���C�_�[�P�ʂ̏Փ˔���
		for (auto& colA : *a) {
			for (auto& colB : *b) {
				// �X�^�e�B�b�N�R���C�_�[���m�͏Փ˂��Ȃ�
				if (colA.origin->isStatic && colB.origin->isStatic) {
					continue;
				}

				bool makeDetection = true;	// ������s����
				// �Փ˔�����s��Ȃ��g�ݍ��킹�ł͂Ȃ������ׂ�
				for (const auto& typePair : NonCollidablePairs) {
					if (colA.origin->type == typePair.first) {
						if (colB.origin->type == typePair.second) {
							makeDetection = false;
							break;
						}
					}
					if (colA.origin->type == typePair.second) {
						if (colB.origin->type == typePair.first) {
							makeDetection = false;
							break;
						}
					}
				}

				// �Փ˔�����s����
				if (!makeDetection) continue;

				const int typeA = static_cast<int>(colA.origin->GetType());
				const int typeB = static_cast<int>(colB.origin->GetType());
				ContactPoint contactPoint;

				// �Փ˔�����s��
				if (funcList[typeA][typeB](colA.world, colB.world, contactPoint)) {
					//���ꂼ��̃R���C�_�[�����L����Q�[���I�u�W�F�N�g���擾
					GameObject* gObjA = colA.origin->GetOwner();
					GameObject* gObjB = colB.origin->GetOwner();

					// �R���C�_�[���d�Ȃ�Ȃ��悤�ɍ��W�𒲐�
					if (!colA.origin->isTrigger && !colB.origin->isTrigger) {
						if (colA.origin->isStatic || 
							colA.origin->priority > colB.origin->priority) {
							// A�͓����Ȃ��̂�B���ړ�������
							ApplyPenetration(
								b, gObjB, contactPoint.penetration);
						}
						else if (colB.origin->isStatic || 
								 colA.origin->priority < colB.origin->priority) {
							// B�͓����Ȃ��̂�A���ړ�������
							ApplyPenetration(
								a, gObjA, -contactPoint.penetration);
						}
						else {
							// A��B���ϓ��Ɉړ�������
							ApplyPenetration(
								b, gObjB, contactPoint.penetration * 0.5f);
							ApplyPenetration(
								a, gObjA, contactPoint.penetration * -0.5f);
						}
					}

					// �C�x���g�֐��̌Ăяo��
					gObjA->OnCollisionHit(colA.origin, colB.origin, contactPoint);
					gObjB->OnCollisionHit(colB.origin, colA.origin, contactPoint);

					// �C�x���g�̌��ʁA�ǂ��炩�̃Q�[���I�u�W�F�N�g���j�����ꂽ�烋�[�v�I��
					if (gObjA->IsDestroyed() || gObjB->IsDestroyed()) {
						return; // �֐����I��
					}
				}
			} // for colB
		} // for colA
	}
}

/**
* �R���C�_�[�̏Փ˔���
*
* @param colliders			���肷��R���C�_�[�̔z��
*/
void ColliderDetection(
	std::vector<WorldColliderList>* colliders)
{
	//���肷��R���C�_�[(���肷��Q�[���I�u�W�F�N�g�̐�)���Q�����Ȃ画�肵�Ȃ�
	if (colliders->size() < 2) {
		return;
	}

	//���肷��R���C�_�[(���肷��Q�[���I�u�W�F�N�g�̐�)�Q�ȏ�

	// �Q�[���I�u�W�F�N�g�P�ʂ̏Փ˔���
	for (auto a = colliders->begin(); a != colliders->end() - 1; ++a) {
		if (a->empty()) {
			// ���肷��R���C�_�[���Ȃ��������΂�
			continue;
		}
		//�R���C�_�[�����L����Q�[���I�u�W�F�N�g���擾
		const GameObject* goA = a->at(0).origin->GetOwner();

		if (goA->IsDestroyed()) {
			continue; // �폜�ς݂Ȃ̂Ŕ�΂�
		}

		for (auto b = a + 1; b != colliders->end(); ++b) {
			if (b->empty()) {
				// ���肷��R���C�_�[���Ȃ��������΂�
				continue;
			}
			//�R���C�_�[�����L����Q�[���I�u�W�F�N�g���擾
			const GameObject* goB = b->at(0).origin->GetOwner();

			if (goB->IsDestroyed()) {
				continue; // �폜�ς݂Ȃ̂Ŕ�΂�
			}

			// �R���C�_�[�P�ʂ̏Փ˔���
			HandleWorldColliderCollision(&*a, &*b);  /*�u&*�v�C�e���[�^����|�C���^�ɕϊ�*/

		} // for b
	} // for a
}