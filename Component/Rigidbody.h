/**
* @file Rigidbody.h
*/
#ifndef RIGIDBODY_H_INCLUDED
#define RIGIDBODY_H_INCLUDED
#include "../Engine/Component.h"
#include "../Engine/GameObject.h"

/**
* ゲームオブジェクトの速度と重力を扱うコンポーネント
*/
class Rigidbody : public Component
{
public:
	static constexpr float gravity = 9.81f; // 重力加速度
	float gravityScale = 1; // 重力の影響を制御する係数
	vec3 velocity = { 0, 0, 0 }; // 速度

	bool isGrounded = false; // 足場となる物体の上に乗っているとtrueになる

public:
	// コンストラクタ・デストラクタ
	Rigidbody()
		:Component("Rigidbody")
	{};
	virtual ~Rigidbody() = default;

	virtual void Update(float deltaSeconds) override {
		GameObject* owner = GetOwner();

		if (isGrounded) {
			// 接地フラグが立っていたら、加速度を0にする
			velocity.y = std::max(velocity.y, 0.0f);
		}
		else {
			// 重力加速度によって速度を更新する
			velocity.y -= gravity * gravityScale * deltaSeconds;
		}

		// 速度を座標に反映する
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