/**
* @file Raycast.h
*/
#ifndef RAYCAST_H_INCLUDED
#define RAYCAST_H_INCLUDED
#include "../Math/MyMath.h"
#include "../Engine/Collision/Collision.h"
#include "../Component/Collider.h"
#include <functional>

// ��s�錾
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
using GameObjectList = std::vector<GameObjectPtr>;

/**
* ����
*/
struct Ray
{
    vec3 start;     // �����̎n�_
    vec3 direction; // �����̌���
};

// �����̌������茋��
struct RayHitInfo
{
    ColliderPtr collider;     // �ŏ��Ɍ����ƌ��������R���C�_�[
    vec3 point = vec3(0);     // �ŏ��̌�_�̍��W
    float distance = 0;       // �ŏ��̌�_�܂ł̋���
};

/**
* ��������̑ΏۂɂȂ邩�ǂ����𒲂ׂ�q��^
*
* @param collider ���肷��R���C�_�[
* @param distance �����ƃR���C�_�[�̌�_�܂ł̋���
*
* @retval true  ��������̑ΏۂɂȂ�
* @retval false ��������̑ΏۂɂȂ�Ȃ�
*/
using RaycastPredicate =
    std::function<bool(const ColliderPtr& collider, float distance)>;


// �e�R���C�_�[�Ƃ̌�������֐�
bool Intersect(const AABB& a,   const Ray& b, float& distance);
bool Intersect(const Sphere& a, const Ray& b, float& distance);
bool Intersect(const Box& a,    const Ray& b, float& distance);


/**
* �}�E�X���W���甭�˂����������擾����(�w�肵���J����)
*
* @param mainCmaera �J�����I�u�W�F�N�g
* 
* @return �}�E�X���W���甭�˂�������
*/
Ray GetRayFromMousePosition(GameObject* camera);


/**
* �����ƃR���C�_�[�̌�������
*
* @param ray            ����
* @param collider       �ڐG���������R���C�_�[�|�C���^
* @param hitInfo        �����ƍŏ��Ɍ��������R���C�_�[�̏��
* @param pred           ����������s���R���C�_�[��I�ʂ���q��
*
* @retval true  �R���C�_�[�ƌ�������
* @retval false �ǂ̃R���C�_�[�Ƃ��������Ȃ�����
*/
bool Raycast(const Ray& ray, const ColliderPtr& collider, 
    RayHitInfo& hitInfo, const RaycastPredicate& pred);

/**
* �����ƃR���C�_�[���X�g�̌�������
*
* @param ray            ����
* @param colliders      �ڐG���������R���C�_�[�z��
* @param hitInfo        �����ƍŏ��Ɍ��������R���C�_�[�̏��
* @param pred           ����������s���R���C�_�[��I�ʂ���q��
*
* @retval true  �R���C�_�[�ƌ�������
* @retval false �ǂ̃R���C�_�[�Ƃ��������Ȃ�����
*/
bool Raycast(const Ray& ray, const ColliderList& colliders,
    RayHitInfo& hitInfo, const RaycastPredicate& pred);

/**
* �����ƃQ�[���I�u�W�F�N�g�R���C�_�[�̌�������
*
* @param ray            ����
* @param gameObjects    �ڐG���������Q�[���I�u�W�F�N�g�z��
* @param hitInfo        �����ƍŏ��Ɍ��������R���C�_�[�̏��
* @param pred           ����������s���R���C�_�[��I�ʂ���q��
*
* @retval true  �R���C�_�[�ƌ�������
* @retval false �ǂ̃R���C�_�[�Ƃ��������Ȃ�����
*/
bool Raycast(const Ray& ray, const GameObjectList& gameObjects, 
    RayHitInfo& hitInfo, const RaycastPredicate& pred);


#endif  // RAYCAST_H_INCLUDED