/**
* @file Raycast.cpp
*/
#include "Raycast.h"

#include "../Engine/Input/InputManager.h"
#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"
#include "../Component/AabbCollider.h"
#include "../Component/SphereCollider.h"
#include "../Component/BoxCollider.h"

namespace {
	/**
	* �X���u(���鎲�ɐ�����2���ʂɈ͂܂ꂽ�͈�)�ƌ����̌�������
	*
	* @param[in]  min       �X���u�̊J�n����
	* @param[in]  max       �X���u�̏I������
	* @param[in]  start     �����̔��˓_
	* @param[in]  direction �����̌���
	* @param[out] tmin      AABB�ƌ����̌����J�n����
	* @param[out] tmax      AABB�ƌ����̌����I������
	*
	* @retval true  �������Ă���
	* @retval false �������Ă��Ȃ�
	*/
	bool IntersectSlab(float min, float max, float start, float direction, float& tmin, float& tmax)
	{
		// �������X���u�ƕ��s�ȏꍇ
		// ���˓_���X���u���ɂ���Ό������Ă���A�O�ɂ���Ό������Ă��Ȃ�
		if (abs(direction) < 0.0001f) {
			return (start >= min) && (start <= max);
		}

		// �����ƃX���u����������J�n�����ƏI�����������߂�
		float t0 = (min - start) / direction;
		float t1 = (max - start) / direction;

		// �����̑��������J�n�����Ƃ���
		//�������}�C�i�X�����Ɍ������Ă���ꍇ�ɊJ�n�_�E�I���_���t�ɂȂ�̂�h��
		if (t0 > t1) {
			std::swap(t0, t1);
		}

		// ���ʂ̌����͈͂����߂�
		// �ȑO�̊J�n�����ƍ���̊J�n�������r���A�x���ق���I��
		if (t0 > tmin) {
			tmin = t0;
		}

		// ���ʂ̌����͈͂����߂�
		// �ȑO�̏I�������ƍ���̏I���������r���A�����ق���I��
		if (t1 < tmax) {
			tmax = t1;
		}

		// �u�J�n���� <= �I�������v�̏ꍇ�͌������Ă���
		return tmin <= tmax;
	}

	/**
	* �X���u(���鎲�ɐ�����2���ʂɈ͂܂ꂽ�͈�)�ƌ����̌�������
	*
	* @param[in]  axis      �X���u�̎��x�N�g��
	* @param[in]  scale     �X���u�̕�
	* @param[in]  start     �����̔��˓_(�X���u�̒��S�����_�Ƃ���)
	* @param[in]  direction �����̌���
	* @param[out] tmin      AABB�ƌ����̌����J�n����
	* @param[out] tmax      AABB�ƌ����̌����I������
	*
	* @retval true  �������Ă���
	* @retval false �������Ă��Ȃ�
	*/
	bool IntersectSlab(const vec3& axis, float scale, const vec3& start, const vec3& direction,
		float& tmin, float& tmax)
	{
		// �����x�N�g���Ɣ��˓_�ɂ��āA���x�N�g�������̐��������߂�
		const float e = dot(axis, direction);	// WHAT?
		const float f = dot(axis, start);

		// �������X���u�ƕ��s�ȏꍇ
		// ���˓_���X���u���ɂ���Ό������Ă���A�O�ɂ���Ό������Ă��Ȃ�
		if (abs(e) < 0.0001f) {
			return (f >= -scale) && (f <= scale);
		}

		// �����ƃX���u�������͈͂̊J�n�����ƏI�����������߂�
		float t0 = (-scale - f) / e;
		float t1 = (scale - f) / e;

		// �����̑��������J�n�����Ƃ���
		if (t0 > t1) {
			const float tmp = t0;
			t0 = t1;
			t1 = tmp;
		}
		
		// �ȑO�̊J�n�����ƍ���̊J�n�������r���A�x���ق���I��
		if (t0 > tmin) {
			tmin = t0;
		}
		
		// �ȑO�̏I�������ƍ���̏I���������r���A�����ق���I��
		if (t1 < tmax) {
			tmax = t1;
		}
		
		// �u�J�n���� <= �I�������v�̏ꍇ�͌������Ă���
		return tmin <= tmax;
	}


	/**
	* �����ƃR���C�_�[�̌�����������s
	*/
	bool ExecutRaycast(const Ray& ray, const ColliderPtr& collider, float& distance)
	{
		// �R���C�_�[�����[���h���W�n�ɕϊ�
		const auto worldCollider =
			collider->GetTransformedCollider(collider->GetOwner()->GetWorldTransformMatrix());

		// �����Ƃ̌�������
		float d;
		bool hit = false;
		switch (collider->GetType()) {
		case Collider::Type::AABB:
			hit = Intersect(static_cast<AabbCollider&>(*worldCollider).aabb, ray, d);
			break;

		case Collider::Type::Sphere:
			hit = Intersect(static_cast<SphereCollider&>(*worldCollider).sphere, ray, d);
			break;

		case Collider::Type::Box:
			hit = Intersect(static_cast<BoxCollider&>(*worldCollider).box, ray, d);
			break;
		}

		distance = d;
		return hit;
	}
}

/**
* AABB�ƌ����̌�������
*
* @param[in]  aabb     ����Ώۂ�AABB
* @param[in]  ray      ����Ώۂ̌���
* @param[out] distance ������AABB�ƍŏ��Ɍ������鋗��
*
* @retval true  �������Ă���
* @retval false �������Ă��Ȃ�
*/
bool Intersect(const AABB& aabb, const Ray& ray, float& distance)
{
	// ���ʂ̌����͈�
	float tmin = 0;
	float tmax = FLT_MAX;

	// �X���u�Ƃ̌�������
	for (int i = 0; i < 3; ++i) {
		if (!IntersectSlab(aabb.min[i], aabb.max[i],
			ray.start[i], ray.direction[i], tmin, tmax)) {
			return false; // �������Ă��Ȃ�
		}
	}

	// ��_�܂ł̋�����ݒ�
	distance = tmin;
	return true; // �������Ă���
}

/**
* ���̂ƌ����̌�������
*
* @param[in]  sphere   ����Ώۂ̋���
* @param[in]  ray      ����Ώۂ̌���
* @param[out] distance ���������̂ƍŏ��Ɍ������鋗��
*
* @retval true  �������Ă���
* @retval false �������Ă��Ȃ�
*/
bool Intersect(const Sphere& sphere, const Ray& ray, float& distance)
{
	const vec3 m = ray.start - sphere.position;
	const float b = dot(m, ray.direction);
	const float c = dot(m, m) - sphere.radius * sphere.radius;

	// �����̎n�_�����̂̊O�ɂ���(c > 0)�A���������̂��痣��Ă���������
	// ���˂��ꂽ(b > 0)�ꍇ�A���̂ƌ����͌������Ȃ�
	if (c > 0 && b > 0) {
		return false;
	}

	// ���ʎ������̏ꍇ�͌������Ȃ�
	const float disc = b * b - c; // ���ʎ�
	if (disc < 0) {
		return false;
	}

	// �ŏ��Ɍ�������ʒu���v�Z
	distance = -b - sqrt(disc);

	// ���̈ʒu�͎n�_����O���w���A���������̓����甭�˂��ꂽ���Ƃ��Ӗ�����
	// ���̏ꍇ�A�n�_���u�ŏ��Ɍ�������ʒu�v�Ƃ���
	if (distance < 0) {
		distance = 0;
	}

	return true;
}

/**
* OBB�ƌ����̌�������
*
* @param[in]  box      ����Ώۂ�OBB
* @param[in]  ray      ����Ώۂ̌���
* @param[out] distance ������OBB�ƍŏ��Ɍ������鋗��
* 
* @retval true  �������Ă���
* @retval false �������Ă��Ȃ�
*/
bool Intersect(const Box& box, const Ray& ray, float& distance)
{
	// �X���u���S�����_�Ƃ����ꍇ�̌����̔��˓_���v�Z
	const vec3 start = ray.start - box.position;

	// �X���u�Ƃ̌�������
	float tmin = 0;
	float tmax = FLT_MAX;
	for (int i = 0; i < 3; ++i) {
		if (!IntersectSlab(box.axis[i], box.scale[i], start, ray.direction,
			tmin, tmax)) {
			return false; // �������Ă��Ȃ�
		}
	}

	// ��_�܂ł̋�����ݒ�
	distance = tmin;
	return true; // �������Ă���
}


/**
* �}�E�X���W���甭�˂����������擾����
*/
Ray GetRayFromMousePosition(GameObject* camera)
{
	// �}�E�X���W(�J�������W�n(Z=-1))���擾
	vec2 mousePos = InputManager::GetMouse()->GetMousePosition();

	// NDC���W�n�ɕϊ����쐬
	vec3 nearPos(mousePos, -1);
	vec3 farPos(mousePos, 1);

	const auto cComp = camera->GetComponent<Camera>();

	// NDC���W�n����N���b�v���W�n�ɕϊ�
	nearPos *= cComp->near;
	farPos *= cComp->far;
	nearPos.z = (nearPos.z - cComp->A) / cComp->B;
	farPos.z = (farPos.z - cComp->A) / cComp->B;

	// �N���b�v���W�n����r���[���W�n�ɕϊ�
	const float invFovScale = 1.0f / cComp->fovScale.y;	/*�v�Z�̏��Z�����炷���߂ɋt���ɂ���*/
	nearPos *= invFovScale;
	farPos *= invFovScale;

	// �r���[���W�n���烏�[���h���W�n�ɕϊ�
	const float cameraSinY = std::sin(camera->rotation.y);
	const float cameraCosY = std::cos(camera->rotation.y);

	//�����̎n�܂�(nearPos)�ƏI���(farPos)�����߂�
	nearPos = {
		nearPos.x * cameraCosY - cComp->near * cameraSinY,
		nearPos.y,
		nearPos.x * -cameraSinY - cComp->near * cameraCosY
	};
	nearPos += camera->position;

	farPos = {
		farPos.x * cameraCosY - cComp->far * cameraSinY,
		farPos.y,
		farPos.x * -cameraSinY - cComp->far * cameraCosY
	};
	farPos += camera->position;

	// �ߕ��ʂ̍��W�Ɖ����ʂ̍��W��������̌����x�N�g�������߂�
	/*�x�N�g���̐��K�����s���Ō�ɂ܂Ƃ߂Ċ��邱�Ƃŏ��Z�����炵�A���������������Ă���*/
	vec3 direction = normalize(farPos - nearPos);

	return Ray{ nearPos, direction };
}


/**
* �����ƃR���C�_�[�|�C���^
*/
bool Raycast(
	const Ray& ray, const ColliderPtr& collider,
	RayHitInfo& hitInfo, const RaycastPredicate& pred)
{
	// ��_�̏���������
	hitInfo.collider = nullptr;
	hitInfo.distance = FLT_MAX;

	float d;
	if (!ExecutRaycast(ray, collider, d)) {
		// �������Ă��Ȃ�
		return false;
	}

	if (!pred(collider, d)) {
		// ��������̑ΏۂłȂ�
		return false;
	}

	hitInfo.collider = collider;
	hitInfo.distance = d;
	hitInfo.point = ray.start + ray.direction * hitInfo.distance;

	return true;
}

/**
* �����ƃR���C�_�[���X�g
*/
bool Raycast(
	const Ray& ray, const ColliderList& colliders,
	RayHitInfo& hitInfo, const RaycastPredicate& pred)
{
	// ��_�̏���������
	hitInfo.collider = nullptr;
	hitInfo.distance = FLT_MAX;

	for (const auto& col : colliders) {

		// ������������s
		float d;
		if (!ExecutRaycast(ray, col, d)) {
			// �������Ă��Ȃ�
			continue;
		}

		// ��������̑ΏۂłȂ���Δ�΂�
		if (!pred(col, d)) {
			continue;
		}

		// ��蔭�˓_�ɋ߂���_�����R���C�_�[��I��
		if (d < hitInfo.distance) {
			hitInfo.collider = col;
			hitInfo.distance = d;
		}

	} // for colliders

	// ��������R���C�_�[��������true�A�Ȃ����false��Ԃ�
	if (hitInfo.collider) {
		// ��_�̍��W���v�Z
		hitInfo.point = ray.start + ray.direction * hitInfo.distance;
		return true;
	}
	return false;
}

/**
* �����ƃQ�[���I�u�W�F�N�g�R���C�_�[
*/
bool Raycast(
	const Ray& ray, const GameObjectList& gameObjects, 
	RayHitInfo& hitInfo, const RaycastPredicate& pred)
{
	// ��_�̏���������
	hitInfo.collider = nullptr;
	hitInfo.distance = FLT_MAX;

	for (const auto& gObj : gameObjects) {	/*Unity�͂����ƌ��������ōs���Ă���*/
		for (const auto& col : gObj->GetColliders()) {
			// ������������s
			float d;
			if (!ExecutRaycast(ray, col, d)) {
				// �������Ă��Ȃ�
				continue;
			}

			// ��������̑ΏۂłȂ���Δ�΂�
			if (!pred(col, d)) {
				continue;
			}

			// ��蔭�˓_�ɋ߂���_�����R���C�_�[��I��
			if (d < hitInfo.distance) {
				hitInfo.collider = col;
				hitInfo.distance = d;
			}

		} // for colliders
	} // for gameObjects

	// ��������R���C�_�[��������true�A�Ȃ����false��Ԃ�
	if (hitInfo.collider) {
		// ��_�̍��W���v�Z
		hitInfo.point = ray.start + ray.direction * hitInfo.distance;
		return true;
	}
	return false;
}