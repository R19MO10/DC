/**
* @file Rigidbody.h
*/
#ifndef RIGIDBODY_H_INCLUDED
#define RIGIDBODY_H_INCLUDED
#include "../Engine/Component.h"
#include "../Engine/GameObject.h"

/**
* �Q�[���I�u�W�F�N�g�̑��x�Əd�͂������R���|�[�l���g
*/
class Rigidbody : public Component
{
public:
	static constexpr float gravity = 9.81f; // �d�͉����x
	float gravityScale = 1; // �d�͂̉e���𐧌䂷��W��
	vec3 velocity = { 0, 0, 0 }; // ���x

	bool isGrounded = false; // ����ƂȂ镨�̂̏�ɏ���Ă����true�ɂȂ�

public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Rigidbody()
		:Component("Rigidbody")
	{};
	virtual ~Rigidbody() = default;

	virtual void Update(float deltaSeconds) override {
		GameObject* owner = GetOwner();

		if (isGrounded) {
			// �ڒn�t���O�������Ă�����A�����x��0�ɂ���
			velocity.y = std::max(velocity.y, 0.0f);
		}
		else {
			// �d�͉����x�ɂ���đ��x���X�V����
			velocity.y -= gravity * gravityScale * deltaSeconds;
		}

		// ���x�����W�ɔ��f����
		owner->position += velocity * deltaSeconds;
	}

	virtual void DrawImGui() override {
		ImGui::DragFloat ("Gravity Scale",	&gravityScale);
		ImGui::Spacing();
		ImGui::DragFloat3("Velocity",		&velocity.x);
		ImGui::Spacing();
		ImGui::Spacing();
		bool g = isGrounded;
		ImGui::Checkbox("Grounded", &g);
	}
};
using RigidbodyPtr = std::shared_ptr<Rigidbody>;

#endif // RIGIDBODY_H_INCLUDED