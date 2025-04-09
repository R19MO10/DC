/**
* InsidePoint.h
*/
#ifndef INSIDEPOINT_H_INCLUDED
#define INSIDEPOINT_H_INCLUDED

#include "../Math/MyMath.h"
#include "../Engine/Collision/Collision.h"
#include "../Component/Collider.h"
#include <functional>

// ��s�錾
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
using GameObjectList = std::vector<GameObjectPtr>;


/**
* �ڐG����̑ΏۂɂȂ邩�ǂ����𒲂ׂ�q��^
*
* @param collider ���肷��R���C�_�[
*
* @retval true  �ڐG����̑ΏۂɂȂ�
* @retval false �ڐG����̑ΏۂɂȂ�Ȃ�
*/
using InsidePointPredicate =
    std::function<bool(const ColliderPtr& collider)>;


// �e�R���C�_�[�Ƃ̐ڐG����֐�
bool Inside(const AABB& aabb,       const vec3 point);
bool Inside(const Sphere& sphere,   const vec3 point);
bool Inside(const Box& box,         const vec3 point);

/**
* �_�ƃR���C�_�[�̐ڐG����
*
* @param point          �_�̍��W
* @param collider       �ڐG���������R���C�_�[�|�C���^
* @param pred           �ڐG������s���R���C�_�[��I�ʂ���q��
* 
* @retval true  �R���C�_�[�ƐڐG����
* @retval false �ǂ̃R���C�_�[�Ƃ��ڐG���Ȃ�����
*/
bool InsidePoint(
    const vec3& point, const ColliderPtr& collider, const InsidePointPredicate& pred);

/**
* �_�ƃR���C�_�[���X�g�̐ڐG����
*
* @param point          �_�̍��W
* @param colliders      �ڐG���������R���C�_�[�|�C���^
* @param pred           �ڐG������s���R���C�_�[��I�ʂ���q��
*
* @return �_�ƐڐG���Ă���R���C�_�[�|�C���^�z��
*/
ColliderList InsidePoint(
    const vec3& point, const ColliderList& colliders, const InsidePointPredicate& pred);


/**
*  �_�ƃQ�[���I�u�W�F�N�g�R���C�_�[�̐ڐG����
*
* @param point          �_�̍��W
* @param gameObjects    �ڐG���������Q�[���I�u�W�F�N�g�z��
* @param pred           ����������s���R���C�_�[��I�ʂ���q��
*
* @return �_�ƐڐG���Ă���R���C�_�[�|�C���^�z��
*/
ColliderList InsidePoint(
    const vec3& point, const GameObjectList& gameObjects, const InsidePointPredicate& pred);

#endif // INSIDEPOINT_H_INCLUDED