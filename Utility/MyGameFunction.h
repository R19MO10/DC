/**
* @file MyGameFunction.h
*/
#ifndef MYGAMEFUCTION_INCLUDE_H
#define MYGAMEFUCTION_INCLUDE_H
#include "../Math/MyMath.h"
#include "Random.h"
#include <memory>

// ��s�錾
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
class Component;
using ComponentPtr = std::shared_ptr<Component>;


// RGB�l��0�`255����0�`1�͈̔͂ɕϊ�����
inline float UnitRangeRGB(const float& color255) {
	return color255 / 255.0f;
}
inline vec3 UnitRangeRGB(const vec3& color255) {
	return vec3(color255 / 255.0f);
}
inline vec4 UnitRangeRGB(const vec4& color255) {
	return vec4(color255 / 255.0f);
}

// �^�[�Q�b�g�̕������������߂̉�]���v�Z����
inline vec3 GetRotationToLookAt(const vec3& position, const vec3& target) {
	// ���g����^�[�Q�b�g�ւ̕����x�N�g�����v�Z
	const vec3 direction = normalize(target - position);

	// �����x�N�g������I�C���[�p���v�Z
	float pitch = std::asin(direction.y);
	// Y�����v�Z���邽�߂ɃA�[�N�^���W�F���g���g�p
	float yaw = std::atan2(-direction.x, -direction.z); 

	return { pitch, yaw, 0.0f }; // ���[����0�ɐݒ�i���̕��@�ł͌v�Z����Ȃ��j
}

// 2�̃x�N�g��A��B����p�x(���W�A��)���v�Z����
inline float AngleBetweenVectors(const vec3& a, const vec3& b) 
{
	// �x�N�g��A�ƃx�N�g��B�̓��ς��v�Z
	float dotProduct = dot(a, b);

	// �x�N�g��A�ƃx�N�g��B�̑傫�����v�Z
	float magnitudeA =length(a);
	float magnitudeB =length(b);

	// �A�[�N�R�T�C�����g�p���ă��W�A���Ŋp�x���v�Z
	float angle = std::acos(dotProduct / (magnitudeA * magnitudeB));

	return angle;
}

// ���K�����ꂽ2�̃x�N�g��A��B����p�x(���W�A��)���v�Z����
inline float AngleBetweenNormalizedVectors(const vec3& na, const vec3& nb)
{
	// �x�N�g��A�ƃx�N�g��B�̓��ς��v�Z�i����͐��K�����ꂽ�x�N�g���Ȃ̂ŁA������1�j
	float dotProduct = dot(na, nb);

	// �A�[�N�R�T�C�����g�p���ă��W�A���Ŋp�x���v�Z
	float angle = std::acos(dotProduct);

	return angle;
}

/**
* �^�[�Q�b�g���W�����g�Ƃ̑��ΓI�ȑO��̈ʒu�𔻒�
* 
* @param self			���g�̃Q�[���I�u�W�F�N�g
* @param targetPosition	�^�[�Q�b�g�̍��W
* 
* @return �u1�v�O, �u-1�v��
*/
int RelativeFrontBackDirection(
	const GameObject* self, const vec3 targetPosition);
int RelativeFrontBackDirection(
	const GameObject* self, const GameObject* targetObj);

/**
* �^�[�Q�b�g���W�����g�Ƃ̑��ΓI�ȍ��E�̈ʒu�𔻒�
*
* @param self			���g�̃Q�[���I�u�W�F�N�g
* @param targetPosition	�^�[�Q�b�g�̍��W
*
* @return �u1�v��, �u-1�v�E
*/
int RelativeLeftRightDirection(
	const GameObject* self, const vec3 targetPosition);
int RelativeLeftRightDirection(
	const GameObject* self, const GameObject* targetObj);

/**
* �Q�[���I�u�W�F�N�g���L�������ׂ�
*/
bool isValid(const GameObject* gameObject);
bool isValid(const GameObjectPtr& gameObject);

/**
* �R���|�[�l���g���L�������ׂ�
*/
bool isValid(const Component* component);
bool isValid(const ComponentPtr& component);

#endif // MYGAMEFUCTION_INCLUDE_H