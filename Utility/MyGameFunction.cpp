/**
* @file MyGameFunction.cpp
*/
#include "MyGameFunction.h"

#include "../Engine/GameObject.h"
#include "../Engine/Component.h"

/**
* �^�[�Q�b�g���W�����g�Ƃ̑��ΓI�ȑO��̈ʒu�𔻒�
*
* @param self			���g�̃Q�[���I�u�W�F�N�g
* @param targetPosition	�^�[�Q�b�g�̍��W
*
* @return �u1�v�O, �u-1�v��
*/
int RelativeFrontBackDirection(
	const GameObject* self, const vec3 targetPosition)
{
	// ���g�̉E���ʃx�N�g��
	vec3 ownerForward =
		normalize(vec3(sin(self->rotation.y), 0.0f, cos(self->rotation.y))) - halfPI;

	// ���g���瑊��܂ł̕����x�N�g��
	vec3 toOtherDirection = normalize(targetPosition - self->position);
	toOtherDirection.y = 0.0f;	// �����x�N�g����XZ���ʂɂ���

	// ���g�̉E���ʃx�N�g���Ƒ���ւ̕����x�N�g���̊O�ς��v�Z
	vec3 crossDirec = cross(ownerForward, toOtherDirection);

	return (crossDirec.y > 0 ? 1 : -1);	// 1�F�O,  -1�F��
}
int RelativeFrontBackDirection(
	const GameObject* self, const GameObject* targetObj)
{
	return RelativeFrontBackDirection(self, targetObj->position);
}


/**
* �^�[�Q�b�g���W�����g�Ƃ̑��ΓI�ȍ��E�̈ʒu�𔻒�
*
* @param self			���g�̃Q�[���I�u�W�F�N�g
* @param targetPosition	�^�[�Q�b�g�̍��W
*
* @return �u1�v��, �u-1�v�E
*/
int RelativeLeftRightDirection(
	const GameObject* self, const vec3 targetPosition)
{
	// ���g�̐��ʃx�N�g��
	vec3 ownerForward =
		normalize(vec3(sin(self->rotation.y), 0.0f, cos(self->rotation.y)));

	// ���g���瑊��܂ł̕����x�N�g��
	vec3 toOtherDirection = normalize(targetPosition - self->position);
	toOtherDirection.y = 0.0f;	// �����x�N�g����XZ���ʂɂ���

	// ���g�̉E���ʃx�N�g���Ƒ���ւ̕����x�N�g���̊O�ς��v�Z
	vec3 crossDirec = cross(ownerForward, toOtherDirection);

	return (crossDirec.y > 0 ? 1 : -1);	// 1�F��,  -1�F�E
}
int RelativeLeftRightDirection(
	const GameObject* self, const GameObject* targetObj)
{
	return RelativeLeftRightDirection(self, targetObj->position);
}

/**
* �Q�[���I�u�W�F�N�g���L�������ׂ�
*/
bool isValid(const GameObject* gameObject) 
{
	return (gameObject && !gameObject->IsDestroyed());
}
bool isValid(const GameObjectPtr& gameObject)
{
	return (isValid(gameObject.get()));
}

/**
* �R���|�[�l���g���L�������ׂ�
*/
bool isValid(const Component* component) 
{
	return (component && !component->IsDestroyed());
}
bool isValid(const ComponentPtr& component)
{
	return (isValid(component.get()));
}