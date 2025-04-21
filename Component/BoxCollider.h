/**
* @file BoxCollider.h
*/
#ifndef BOXCOLLIDER_H_INCLUDED
#define BOXCOLLIDER_H_INCLUDED
#include "Collider.h"

/**
* BOX(OBB)�R���C�_�[
*/
class BoxCollider : public Collider
{
public:
	Box box = {
		{ 0, 0, 0 },
		{ { 1, 0, 0}, { 0, 1, 0 }, { 0, 0, 1} },
		{ 1, 1, 1 } 
	};

public:
	BoxCollider() 
		:Collider("BoxCollider")
	{};
	virtual ~BoxCollider() = default;

	// �}�`�̎��
	inline Type GetType() const override { return Type::Box; }

	// ���W��ύX����
	inline void AddPosition(const vec3& translate) override
	{
		box.position += translate;
	}

	// ���W�ϊ������R���C�_�[���擾����
	inline ColliderPtr GetTransformedCollider(const mat4& transform) const override
	{
		// ���W�ϊ��s��𕪉�
		vec3 translate; // ���g�p
		mat3 rotation;
		vec3 scale;
		Decompose(transform, translate, rotation, scale);
		
		// ���W�ϊ������R�s�[���쐬
		const auto& p = std::make_shared<BoxCollider>();
		p->box.position = vec3(transform * vec4(box.position, 1));
		for (int i = 0; i < 3; ++i) {
			p->box.axis[i]  = rotation * box.axis[i];	// ���x�N�g��
			p->box.scale[i] = scale[i] * box.scale[i];	// �g�嗦
		}
		return p;
	}

	// �}�`���擾����
	inline const Box& GetShape() const { return box; }

public:
	// �R���W������\������
	virtual void DrawCollision(const ProgramPipeline& prog);

public:
	virtual void DrawImGui() override;
};
using BoxColliderPtr = std::shared_ptr<BoxCollider>;

#endif // BOXCOLLIDER_H_INCLUDED