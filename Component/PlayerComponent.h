/**
* @file PlayerComponent.h
*/
#ifndef PLAYER_COMPONENT_H_INCLUDED
#define PLAYER_COMPONENT_H_INCLUDED
#include "CharacterComponent.h"
#include "../Math/MyMath.h"
#include "../Utility/EventDispatcher.h"


// ��s�錾
class GameObject;
using GameObject_WeakPtr = std::weak_ptr<GameObject>;
class CameraStand;
using CameraStandPtr = std::shared_ptr<CameraStand>;
class ScreenEffects;
using ScreenEffectsPtr = std::shared_ptr<ScreenEffects>;
class PlayerStateMachine;
using PlayerStateMachinePtr = std::shared_ptr<PlayerStateMachine>;
class GltfAnimatedMeshRenderer;
using GltfAnimatedMeshRendererPtr = std::shared_ptr<GltfAnimatedMeshRenderer>;
class PlayerHpGauge;
using PlayerHpGaugePtr = std::shared_ptr<PlayerHpGauge>;

/**
* �v���C���[�̃p�����[�^��_���[�W���Ǘ�
*/
class PlayerComponent : public CharacterComponent
{
private:
	CameraStandPtr cameraStand;	// �J�����X�^���h
	PlayerStateMachinePtr playerStateMachine;	// �v���C���[�X�e�[�g�}�V��
	GltfAnimatedMeshRendererPtr animMesh;		// �A�j���[�V�������b�V��

	ScreenEffectsPtr screenEffects;	// �S��ʃG�t�F�N�g
	PlayerHpGaugePtr hpGauge;		// Hp�Q�[�W

	//�v���C���[�����ڂ��Ă���Q�[���I�u�W�F�N�g�iWeakPtr�Ȃ̂͑��ݎQ�Ƃ̉\�������邽�߁j
	GameObject_WeakPtr targetObject;	

private:
	int currentHitPoints = 0;

	bool isTargeted = false;	// �Q�[���I�u�W�F�N�g�ɒ��ڂ��Ă��邩

	bool isPoweredUp = false;			// �p���[�A�b�v���Ă��邩
	float powerUp_elapsedTime_s = 0;	// �p���[�A�b�v�o�ߎ���
	enum PowerUpStep : uint8_t {
		Ready,	// �J�n
		Flash,	// ��������u��������
		Fade,	// ������ʏ�܂Ŗ߂�
		Wave,	// �������E�F�[�u�Ԋu�ɍs��
		FinishStanby,	// �I������
		Finish,	// �I��
	};
	PowerUpStep powerUpStep = Ready;	// �p���[�A�b�v�����̃X�e�b�v

public:
	int maxHitPoints = 4;
	float walkSpeed = 3.0f;
	float runSpeed = 5.0f;
	float jumpPower = 5.5f;
	float damageInterval_s = 1.0f;

	float powerUpDuration_s = 10.0f;	// �p���[�A�b�v�̎�������
	vec4 powerUpEmissionColor =			// �p���[�A�b�v�G�~�b�V���Fs
		vec4(0.008f, 0.016f, 0.2f, 0.5f);

public:
	PlayerComponent()
		:CharacterComponent("PlayerComponent")
	{}
	virtual ~PlayerComponent() = default;

public:
	EventDispatcher<> ed_PowerUp;
	EventDispatcher<> ed_ReceiveDamage;
	EventDispatcher<int> ed_Death;

public:
	// �p���[�A�b�v������
	void PowerUp();		

	// Hp���񕜂�����
	void HealHp(int healHpCnt = 1);

	// ���ڐݒ�
	void SetTargeting(bool isTargeted, const GameObjectPtr& targetObj = nullptr);

private:
	// �p���[�A�b�v�X�V����
	void UpdatePowerUp(float deltaSeconds);

public:
	void AnyDamage(
		float damage, GameObject* damageObject, const ContactPoint& contactPoint) override;

public:
	virtual void Start() override;
	virtual void Update(float deltaSeconds) override;
	virtual void EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat) override;
	virtual void DrawImGui() override;
};
using PlayerComponentPtr = std::shared_ptr<PlayerComponent>;

#endif // PLAYER_COMPONENT_H_INCLUDED