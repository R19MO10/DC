/**
* @file ColliderDetection.h
*/
#ifndef COLLIDER_DETECTION_H_INCLUDED
#define COLLIDER_DETECTION_H_INCLUDED
#include "../../Math/MyMath.h"
#include "../Collision/Collision.h"
#include <memory>
#include <vector>

// ��s�錾
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;


// ���[���h���W�n�̃R���C�_�[��\���\����
struct WorldCollider
{
	ColliderPtr origin;	// �I���W�i���R���C�_�[
	ColliderPtr world;	// ���[���h���W�K����̃R���C�_�[

	// ���W��ύX����
	void AddPosition(const vec3& v);
};
using WorldColliderList = std::vector<WorldCollider>;

/**
* �R���C�_�[�̏Փ˔���
* 
* @param colliders		���肷��R���C�_�[�̔z��
*/
void ColliderDetection(
	std::vector<WorldColliderList>* colliders);

#endif // COLLISION_DETECTION_H_INCLUDED