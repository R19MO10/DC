/**
* @file Light.h
*/
#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include "../Engine/Component.h"
#include "../Math/MyMath.h"
#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"

/**
* ���C�g�R���|�[�l���g
*/
class Light : public Component, 
	public std::enable_shared_from_this<Light> /* ���g�̐��|�C���^����shared_ptr�ɖ߂��Ƃ��Ɏg�p */
{
public:
	// ���C�g�̎��
	enum class Type
	{
		PointLight, // �_����
		SpotLight,  // �X�|�b�g����
	};
	Type type = Type::PointLight; // ���C�g�̎��

	vec3 color = vec3(1); // �F
	float intensity = 1; // ���邳
	float radius = 1;    // ���C�g���͂��ő唼�a

	// �X�|�b�g���C�g�p�̃p�����[�^
	// ������GameObject::rotation����v�Z
	float coneAngle_rad = degrees_to_radians(30);    // �X�|�b�g���C�g���Ƃ炷�p�x
	float falloffAngle_rad = degrees_to_radians(20); // �X�|�b�g���C�g�̌����J�n�p�x

public:
	Light()
		:Component("Light")
	{};
	virtual ~Light() = default;

	virtual void Awake() override {
		// ���C�g�o�b�t�@�Ɏ��g��ǉ�����
		/* ���g��shared_ptr�ɖ߂� */
		GetOwner()->GetEngine()->AllocateLight(shared_from_this());
	}

	virtual void DrawImGui() override {
		static const char* lightTypes[] = { "PointLight", "SpotLight" };
		int typeNum = static_cast<int>(type);
		ImGui::Combo("Type", &typeNum, lightTypes, IM_ARRAYSIZE(lightTypes));
		type = static_cast<Type>(typeNum);
		ImGui::Spacing();
		ImGui::ColorEdit3("Color", &color.x);
		ImGui::DragFloat("Intensity", &intensity);
		ImGui::DragFloat("Radius", &radius);

		if (type == Type::SpotLight) {
			ImGui::Spacing();
			float caDeg = radians_to_degrees(coneAngle_rad);
			ImGui::DragFloat("Cone Angle", &caDeg);
			coneAngle_rad = degrees_to_radians(caDeg);
			float faDeg = radians_to_degrees(falloffAngle_rad);
			ImGui::DragFloat("Falloff Angle", &faDeg);
			falloffAngle_rad = degrees_to_radians(faDeg);
		}

	}
};
using LightPtr = std::shared_ptr<Light>;

#endif // LIGHT_H_INCLUDED