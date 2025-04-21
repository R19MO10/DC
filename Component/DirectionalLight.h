/**
* @file DirectionalLight.h
*/
#ifndef DIRECTIONALLIGHT_H_INCLUDED
#define DIRECTIONALLIGHT_H_INCLUDED
#include "../Engine/Component.h"

/**
* 平行光源コンポーネント
*/
class DirectionalLight : public Component
{
private:
	float preIntensity = 0;	// 変更前の明るさ値
	vec3 direction;			// 平行光源の方向ベクトル

public:
	vec3 color = { 1.00f, 0.98f, 0.95f }; // 色
	float intensity = 5.0f;	// 明るさ

public:
	DirectionalLight()
		: Component("DirectionalLight")
	{}
	virtual ~DirectionalLight() = default;

	inline const vec3 GetDirection() const { return direction; }

	/**
	* 暗転を行う
	* 
	* @param intensity 暗転時の明るさ
	*/
	inline void Blackout(float intensity = 0.5f) {
		preIntensity = this->intensity;
		this->intensity = intensity;
	}

	/**
	* 明転を行う
	* 
	* @param intensity 目標の明るさ(0なら暗転前の値を適応する)
	*/
	inline void Bright(float intensity = 0) {
		// 設定する明るさ
		if (intensity <= 0) {
			// 0以下 の場合は変更前の明るさに
			this->intensity = preIntensity;
		}
		else {
			// 新たに設定された明るさに
			this->intensity = intensity;
		}
	}

public:
	virtual void EndUpdate(
		float deltaSeconds, mat4& ownerWorldTransMat) override 
	{
		// 方向ベクトル(平行光源方向)を設定
		/* オブジェクトの正面ベクトルを方向とする */
		direction = normalize(vec3(ownerWorldTransMat[2]));
	}

	virtual void DrawImGui() override {
		ImGui::DragFloat3("Color", &color.x);
		ImGui::DragFloat("Intensity", &intensity);
	}
};
class DirectionalLight;
using DirectionalLightPtr = std::shared_ptr<DirectionalLight>;

#endif // DIRECTIONALLIGHT_H_INCLUDED
