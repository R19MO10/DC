/**
* @file Component.h
*/
#ifndef COMOPNENT_H_INCLUDED
#define COMOPNENT_H_INCLUDED
#include "../Math/MyMath.h"
#include <imgui.h>
#include <string>
#include <vector>
#include <memory>

// ��s�錾
struct ContactPoint;
class Engine;
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;
class Component;
using ComponentPtr = std::shared_ptr<Component>; // �R���|�[�l���g�̃|�C���^
using ComponentList = std::vector<ComponentPtr>; // �R���|�[�l���g�|�C���^�z��

/**
* �R���|�[�l���g�̊��N���X
*/
class Component
{
	friend Engine;
	friend GameObject;
public:
	std::string name;	// �R���|�[�l���g�̖��O(ImGui�\���ɂ����g�p���Ȃ�����string�^���g�p)
	bool isActive = true;	// �R���|�[�l���g�̊������

private:
	GameObject* owner = nullptr; // ���̃R���|�[�l���g�̏��L��
	bool isStarted = false;      // Start�����s���ꂽ��true�ɂȂ�
	bool isDestroyed = false;    // Destroy�����s���ꂽ��true�ɂȂ�

public:
	Component(const char* compName = "Defult Component")
		:name(compName)
	{};
	virtual ~Component() = default;

	// �R���|�[�l���g�̏��L�҂��擾
	inline GameObject* GetOwner() const { return owner; }

	// �R���|�[�l���g���Q�[���I�u�W�F�N�g����폜����
	inline void Destroy() { isDestroyed = true; }

	// �R���|�[�l���g���j�󂳂�Ă�����true��Ԃ��i�j����Ԃ������j
	inline bool IsDestroyed() const { return isDestroyed; }

public:
	// �Q�[���I�u�W�F�N�g�ɒǉ����ꂽ���ɌĂяo�����
	virtual void Awake() {}

	// �ŏ���Update�̒��O�ŌĂяo�����
	virtual void Start() {}

	// ���t���[��1��Ăяo�����
	virtual void Update(float deltaSeconds) {}
	
	// ���[���h���W�v�Z��̏���
	virtual void EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat) {}

	// �Փ˂��N�����Ƃ��ɌĂяo�����
	virtual void OnCollisionHit(
		const ColliderPtr& self, const ColliderPtr& other, const ContactPoint& contactPoint) {}

	// �폜�����Ƃ��ɌĂяo�����
	virtual void OnDestroy() {}

	// ImGui�ɃR���|�[�l���g�����o��
	virtual void DrawImGui() {}
};

#endif // COMOPNENT_H_INCLUDED