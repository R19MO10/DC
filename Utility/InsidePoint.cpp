/**
* InsidePoint.cpp
*/
#include "InsidePoint.h"

#include "../Engine/GameObject.h"
#include "../Component/AabbCollider.h"
#include "../Component/SphereCollider.h"
#include "../Component/BoxCollider.h"
#include <algorithm>


namespace {
	/**
	* �����ƃR���C�_�[�̐ڐG��������s
	*/
	bool ExecutInsidePoint(const vec3& point, const ColliderPtr& collider)
	{
		// �R���C�_�[�����[���h���W�n�ɕϊ�
		const auto worldCollider =
			collider->GetTransformedCollider(collider->GetOwner()->GetWorldTransformMatrix());

		// �_�Ƃ̐ڐG����
		switch (collider->GetType()) {
		case Collider::Type::AABB:
			return Inside(static_cast<AabbCollider&>(*worldCollider).aabb, point);
			break;

		case Collider::Type::Sphere:
			return Inside(static_cast<SphereCollider&>(*worldCollider).sphere, point);
			break;

		case Collider::Type::Box:
			return Inside(static_cast<BoxCollider&>(*worldCollider).box, point);
			break;
		}

		return false;
	}
}


/**
* AABB�Ɠ_�̐ڐG����
*
* @param[in]  aabb		����Ώۂ̋���
* @param[in]  point		����Ώۂ̓_
*
* @retval true  �ڐG���Ă���
* @retval false �ڐG���Ă��Ȃ�
*/
bool Inside(const AABB& aabb, const vec3 point)
{
	// �ŋߐړ_�܂ł̋�����0����������΁A�ڐG���Ă��Ȃ�
	const vec3 p = Calc::ClosestPoint(aabb, point);
	const vec3 v = point - p;
	const float d2 = dot(v, v);
	if (d2 > 0) {
		return false;
	}
	return true;
}

/**
* ���̂Ɠ_�̐ڐG����
*
* @param[in]  sphere	����Ώۂ̋���
* @param[in]  point		����Ώۂ̓_
*
* @retval true  �ڐG���Ă���
* @retval false �ڐG���Ă��Ȃ�
*/
bool Inside(const Sphere& sphere, const vec3 point)
{
	const vec3 v = sphere.position - point;
	const float d2 = dot(v, v); // v�̒�����2��
	// d2�����̂̔��a��蒷���ꍇ�͐ڐG���Ă��Ȃ�
	if (d2 > sphere.radius * sphere.radius) { // ������������邽�߁A2�擯�m�Ŕ�r����
		return false;
	}
	return true;
}

/**
* OBB�Ɠ_�̐ڐG����
*
* @param[in]  box		����Ώۂ�OBB
* @param[in]  point		����Ώۂ̓_
*
* @retval true  �ڐG���Ă���
* @retval false �ڐG���Ă��Ȃ�
*/
bool Inside(const Box& box, const vec3 point)
{
	// �ŋߐړ_�܂ł̋�����0����������΁A�ڐG���Ă��Ȃ�
	const vec3 p = Calc::ClosestPoint(box, point);
	const vec3 v = point - p;
	const float d2 = dot(v, v);
	if (d2 > 0) {
		return false;
	}
	return true;
}


/**
*  �_�ƃR���C�_�[�̐ڐG����
*/
bool InsidePoint(
	const vec3& point, const ColliderPtr& collider, const InsidePointPredicate& pred)
{
	// �ڐG����̑ΏۂłȂ���Δ�΂�
	/* Raycast�ƈقȂ苗���̌v�Z�͍s��Ȃ����ߐڐG����O�ɍs�������ł��� */
	if (!pred(collider)) {
		return false;
	}

	// �ڐG��������s
	if (!ExecutInsidePoint(point, collider)) {
		return false;
	}

	// �ڐG���Ă���
	return true;
}

/**
* �_�ƃR���C�_�[���X�g�̐ڐG����
*/
ColliderList InsidePoint(
	const vec3& point, const ColliderList& colliders, const InsidePointPredicate& pred)
{
	ColliderList hitColliders;

	for (const auto& col : colliders) {

		// �ڐG����̑ΏۂłȂ���Δ�΂�
		/* Raycast�ƈقȂ苗���̌v�Z�͍s��Ȃ����ߐڐG����O�ɍs�������ł��� */
		if (!pred(col)) {
			continue;
		}

		// �ڐG��������s
		if (!ExecutInsidePoint(point, col)) {
			continue;
		}

		// �ڐG���Ă��邽�ߐڐG�R���C�_�[�z��ɒǉ�����
		hitColliders.push_back(col);
	}

	return hitColliders;
}

/**
*  �_�ƃQ�[���I�u�W�F�N�g�R���C�_�[�̐ڐG����
*/
ColliderList InsidePoint(
	const vec3& point, const GameObjectList& gameObjects,  const InsidePointPredicate& pred)
{	
	ColliderList hitColliders;

	for (const auto& gObj : gameObjects) {
		for (const auto& col : gObj->GetColliders()) {
			// �ڐG����̑ΏۂłȂ���Δ�΂�
			/* Raycast�ƈقȂ苗���̌v�Z�͍s��Ȃ����ߐڐG����O�ɍs�������ł��� */
			if (!pred(col)) {
				continue;
			}

			// �ڐG��������s
			if (!ExecutInsidePoint(point, col)) {
				continue;
			}

			// �ڐG���Ă��邽�ߐڐG�R���C�_�[�z��ɒǉ�����
			hitColliders.push_back(col);
		}
	}

	return hitColliders;
}
