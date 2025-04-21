/**
* @file ShpereCollider.h
*/
#ifndef SPHERECOLLIDER_H_INCLUDED
#define SPHERECOLLIDER_H_INCLUDED
#include "Collider.h"

/**
* ���̃R���C�_�[
*/
class SphereCollider : public Collider
{
public:
	Sphere sphere = { vec3(0), 1 }; // �}�`(����)

public:
	SphereCollider() 
		:Collider("SphereCollider")
	{};
	virtual ~SphereCollider() = default;

	// �}�`�̎��
	inline Type GetType() const override { return Type::Sphere; }

	// ���W��ύX����
	inline void AddPosition(const vec3& translate) override {
		sphere.position += translate;
	}

	// ���W�ϊ������R���C�_�[���擾����
	inline ColliderPtr GetTransformedCollider(const mat4& transform) const override {
		// ���S���W�����W�ϊ�����
		const auto& p = std::make_shared<SphereCollider>();
		p->sphere.position = vec3(transform * vec4(sphere.position, 1));

		// ���̂̊g�嗦�̓I�u�W�F�N�g��XYZ�g�嗦�̂����ő�̂��̂Ƃ���(Unity����)
		const vec3 scale = ExtractScale(transform);
		const float maxScale = std::max({ scale.x, scale.y, scale.z });
		p->sphere.radius = sphere.radius * maxScale;

		return p;
	}

	// �}�`���擾����
	inline const Sphere& GetShape() const { return sphere; }

public:
	// �R���W������\������
	virtual void DrawCollision(const ProgramPipeline& prog);

public:
	virtual void DrawImGui() override;
};
using SphereColliderPtr = std::shared_ptr<SphereCollider>;

#endif // SPHERECOLLIDER_H_INCLUDED