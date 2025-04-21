/**
* @file Collider.h
*/
#ifndef COLLIDER_H_INCLUDED
#define COLLIDER_H_INCLUDED
#include "../Engine/Component.h"
#include "../Engine/Collision/Collision.h"
#include "../Math/MyMath.h"

// ��s�錾
class ProgramPipeline;

/**
* �R���C�_�[�̊��N���X
*/
class Collider : public Component
{
public:
	CollisionType type = CollisionType::Default;
	int priority = 0;		// �Փ˗D��x(�l�������������������Ɠ����������ɉ�����Ȃ�)
	bool isTrigger = false; // true=�d��������, false=�d�����֎~
	bool isStatic = false;  // true=�����Ȃ�����, false=��������

public:
	Collider(const char* compName = "Collider")
		:Component(compName)
	{};
	virtual ~Collider() = default;

public:
	// �}�`�̎��
	enum class Type {
		AABB,   // �����s���E�{�b�N�X
		Sphere, // ����
		Box,    // �L�����E�{�b�N�X
	};
	virtual Type GetType() const = 0;

	// ���W��ύX����
	virtual void AddPosition(const vec3& translate) = 0;

	// ���W�ϊ������R���C�_�[���擾����
	virtual ColliderPtr GetTransformedCollider(const mat4& transform) const = 0;

public:
	// �R���W������`��(�����\��)����
	virtual void DrawCollision(const ProgramPipeline& prog) = 0;
};
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;
using ColliderList = std::vector<ColliderPtr>;

#endif // COLLIDER_H_INCLUDED