/**
* @file CharacterComponent.h
*/
#ifndef CHARACTER_COMPONENT_H_INCLUDED
#define CHARACTER_COMPONENT_H_INCLUDED
#include "../Engine/Component.h"

/**
* �L�����N�^�[�R���|�[�l���g�x�[�X
*/
class CharacterComponent : public Component
{
public:
	float lastDamage_elapsedTime_s = 0;	//�Ō�̃_���[�W����̌o�ߎ���

public:
	CharacterComponent(const char* name = "CharacterComponent")
		: Component(name)
	{}
	virtual ~CharacterComponent() = default;

public:
	/**
	* �_���[�W���󂯂�
	*
	* @param damage			�_���[�W��
	* @param damageObject	���g�Ƀ_���[�W��^�����Q�[���I�u�W�F�N�g�|�C���^
	* @param contactPoint	�Փˏ���ێ�����\����
	*/
	virtual void AnyDamage(
		float damage, GameObject* damageObject, const ContactPoint& contactPoint) = 0;;

public:
	virtual void Update(float deltaSeconds) override {
		lastDamage_elapsedTime_s += deltaSeconds;
	}
};
using CharacterComponentPtr = std::shared_ptr<CharacterComponent>;

#endif // CHARACTER_COMPONENT_H_INCLUDED