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
* ライトコンポーネント
*/
class Light : public Component, 
	public std::enable_shared_from_this<Light> /* 自身の生ポインタからshared_ptrに戻すときに使用 */
{
public:
	// ライトの種類
	enum class Type
	{
		PointLight, // 点光源
		SpotLight,  // スポット光源
	};
	Type type = Type::PointLight; // ライトの種類

	vec3 color = vec3(1); // 色
	float intensity = 1; // 明るさ
	float radius = 1;    // ライトが届く最大半径

	// スポットライト用のパラメータ
	// 方向はGameObject::rotationから計算
	float coneAngle_rad = degrees_to_radians(30);    // スポットライトが照らす角度
	float falloffAngle_rad = degrees_to_radians(20); // スポットライトの減衰開始角度

public:
	Light()
		:Component("Light")
	{};
	virtual ~Light() = default;

	virtual void Awake() override {
		// ライトバッファに自身を追加する
		/* 自身をshared_ptrに戻す */
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