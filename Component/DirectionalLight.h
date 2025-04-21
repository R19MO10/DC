/**
* @file DirectionalLight.h
*/
#ifndef DIRECTIONALLIGHT_H_INCLUDED
#define DIRECTIONALLIGHT_H_INCLUDED
#include "../Engine/Component.h"

/**
* ���s�����R���|�[�l���g
*/
class DirectionalLight : public Component
{
private:
	float preIntensity = 0;	// �ύX�O�̖��邳�l
	vec3 direction;			// ���s�����̕����x�N�g��

public:
	vec3 color = { 1.00f, 0.98f, 0.95f }; // �F
	float intensity = 5.0f;	// ���邳

public:
	DirectionalLight()
		: Component("DirectionalLight")
	{}
	virtual ~DirectionalLight() = default;

	inline const vec3 GetDirection() const { return direction; }

	/**
	* �Ó]���s��
	* 
	* @param intensity �Ó]���̖��邳
	*/
	inline void Blackout(float intensity = 0.5f) {
		preIntensity = this->intensity;
		this->intensity = intensity;
	}

	/**
	* ���]���s��
	* 
	* @param intensity �ڕW�̖��邳(0�Ȃ�Ó]�O�̒l��K������)
	*/
	inline void Bright(float intensity = 0) {
		// �ݒ肷�閾�邳
		if (intensity <= 0) {
			// 0�ȉ� �̏ꍇ�͕ύX�O�̖��邳��
			this->intensity = preIntensity;
		}
		else {
			// �V���ɐݒ肳�ꂽ���邳��
			this->intensity = intensity;
		}
	}

public:
	virtual void EndUpdate(
		float deltaSeconds, mat4& ownerWorldTransMat) override 
	{
		// �����x�N�g��(���s��������)��ݒ�
		/* �I�u�W�F�N�g�̐��ʃx�N�g��������Ƃ��� */
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
