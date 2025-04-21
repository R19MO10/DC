/**
* @file PlayerComponent.cpp
*/
#include "PlayerComponent.h"

#include "../Utility/MyGameFunction.h"
#include "../Engine/EasyAudio/EasyAudio.h"
#include "../Engine/Input/InputSystem.h"

#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"
#include "Collider.h"
#include "CameraStand.h"
#include "ScreenEffects.h"
#include "PlayerStateMachine.h"
#include "GltfAnimatedMeshRenderer.h"
#include "PlayerHpGauge.h"
#include "ParticleSystem.h"

#include "../State/Player/P_DamageState.h"
#include "../State/Player/P_DeathState.h"
#include "../State/MainGame/MG_PlayerLose.h"
#include "../ParticleEffect/PowerUpParticle.h"
#include "../ParticleEffect/PlayerDamageParticle.h"
#include "../ParticleEffect/ArmorBreakParticle.h"
#include "../SoundCue/SC_PlayerPowerUpStart.h"
#include "../SoundCue/SC_PlayerPowerUpFinish.h"
#include "../SoundCue/SC_PlayerArmorGuard.h"
#include "../SoundCue/SC_PlayerDamage.h"


/**
* �p���[�A�b�v������
*/
void PlayerComponent::PowerUp()
{
	// �A�j���[�V�������b�V���ɔ����G�~�b�V������������
	if (isValid(animMesh)) {
		// ������������
		animMesh->materials[0]->emission = powerUpEmissionColor;
	}
	else {
		animMesh.reset();
		Debug::Log(GetFileName(__FILE__) + "Cannot get animMesh");
		return;
	}

	isPoweredUp = true;
	powerUp_elapsedTime_s = 0;
	powerUpStep = PowerUpStep::Ready;

	// �p���[�A�b�v�T�E���h���Đ�
	EasyAudio::PlaySound2D<SC_PlayerPowerUpStart>();

	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();
	
	// �G�t�F�N�g���o��
	const auto& p = engine->Create<GameObject>("PowerUpParticle");
	p->SetParent(owner);
	const auto& ps = p->AddComponent<ParticleSystem>();
	ps->SpawnParticle<PowerUpParticle>();
}

/**
* Hp���񕜂�����
*/
void PlayerComponent::HealHp(int healHpCnt)
{
	currentHitPoints =
		std::min(currentHitPoints + healHpCnt, maxHitPoints);

	// HP�l���Q�[�W�ɔ��f
	if (isValid(hpGauge)) {
		hpGauge->SetHitPoints(currentHitPoints);
	}
	else {
		hpGauge.reset();
		Debug::Log(GetFileName(__FILE__) + "Cannot get hpGauge");
	}
}

/**
* ���ڃ^�[�Q�b�g��ݒ肷��
*/
void PlayerComponent::SetTargeting(bool isTargeted, const GameObjectPtr& targetObj)
{
	this->isTargeted = isTargeted;

	if (isTargeted) {
		if (targetObj) {
			targetObject = targetObj;
		}
		else {
			GameObject* owner = GetOwner();

			/* �G���������ꍇ���l�����Ă����Ă��炩���߃h���S���I�u�W�F�N�g��ݒ肵�Ă��Ȃ� */
			if (const auto& nearEnemy =
				owner->GetEngine()->FindGameObjectWithTag("Dragon"))
			{
				// �ł��߂��G��������ԂȂ�ݒ肷��
				if (nearEnemy->CheckActive()) {
					targetObject = nearEnemy;
				}
				else {
					this->isTargeted = false;
				}
			}
			else {
				this->isTargeted = false;
			}
		}
	}
	else {
		targetObject.reset();
	}
}

/**
* �p���[�A�b�v�X�V����
*/
void PlayerComponent::UpdatePowerUp(float deltaSeconds)
{
	// �p���[�A�b�v���łȂ���Ώ������Ȃ�
	if (!isPoweredUp) {
		return;
	}

	// �A�j���[�V�������b�V�����Ȃ����߃p���[�A�b�v���I������
	if (!isValid(animMesh)) {
		animMesh.reset();

		// �p���[�A�b�v���I������
		isPoweredUp = false;
		// �S��ʃp���[�A�b�v�G�t�F�N�g���I������
		if (screenEffects) {
			screenEffects->FinishPowerUpEffect();
		}

		Debug::Log(GetFileName(__FILE__) +
			"Cannot get animMesh, so power-ups cannot be performed");
		return;
	}


	// �o�ߎ��Ԃ����Z
	powerUp_elapsedTime_s += deltaSeconds;

	// �A�j���[�V�������b�V���ւɃG�~�b�V�����ւ̎Q��
	vec4& emisColor = animMesh->materials[0]->emission;

	switch (powerUpStep)
	{
	case PowerUpStep::Ready:
		if (powerUp_elapsedTime_s > 0.02f) {
			// ������������
			emisColor = powerUpEmissionColor;

			// �v���C���[�X�e�[�g�}�V���̃p�����[�^����������
			if (isValid(playerStateMachine)) {
				// �p�����[�^����
				BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
				blackBoard->SetValue<float>("WalkSpeed", walkSpeed * 1.4f);
				blackBoard->SetValue<float>("RunSpeed", runSpeed * 1.4f);
				blackBoard->SetValue<float>("JumpPower", jumpPower * 1.3f);
			}
			else {
				Debug::Log(GetFileName(__FILE__) +
					"Cannot get playerStateMachine");
			}

			// HP�Q�[�W���p���[�A�b�v�F�ɂ���
			if (isValid(hpGauge)) {
				hpGauge->ChangePowerUpColor(true);
			}
			else {
				Debug::Log(GetFileName(__FILE__) +
					"Cannot get hpGauge");
			}

			ed_ReceiveDamage.DispatchEvent();

			//// �S��ʃp���[�A�b�v�G�t�F�N�g�����s
			//if (isValid(screenEffects)) {
			//	screenEffects->StartPowerUpEffect();
			//}
			//else {
			//	Debug::Log(GetFileName(__FILE__) +
			//		"Cannot get screenEffects");
			//}

			powerUpStep = PowerUpStep::Flash;
		}
		break;

	case PowerUpStep::Flash:
		// �G�~�b�V�������x����u��������
		emisColor.w = std::min(emisColor.w + deltaSeconds * 100, 15.0f);

		if (powerUp_elapsedTime_s > 0.13f) {
			powerUpStep = PowerUpStep::Fade;
		}
		break;

	case PowerUpStep::Fade:
		// �G�~�b�V�������x��ʏ�܂Ŗ߂�
		emisColor.w = std::max(emisColor.w - deltaSeconds * 50, 1.0f);

		if (emisColor.w <= 1.0f) {
			powerUpStep = PowerUpStep::Wave;
		}
		break;

	case PowerUpStep::Wave:
		// �G�~�b�V�������E�F�[�u��Ɏ��s
		emisColor.w = 1.0f + std::sin(powerUp_elapsedTime_s * 2);

		// ���s���Ԃ�����������I��������
		if (powerUp_elapsedTime_s >= powerUpDuration_s) {
			// �p���[�A�b�v�̏I�������Đ�
			EasyAudio::PlaySound2D<SC_PlayerPowerUpFinish>();
			powerUpStep = PowerUpStep::FinishStanby;
		}
		break;

	case PowerUpStep::FinishStanby:
		// �v���C���[�X�e�[�g�}�V���̃p�����[�^���ɖ߂�
		if (isValid(playerStateMachine)) {
			// �p�����[�^��߂�
			BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
			blackBoard->SetValue<float>("WalkSpeed", walkSpeed);
			blackBoard->SetValue<float>("RunSpeed", runSpeed);
			blackBoard->SetValue<float>("JumpPower", jumpPower);
		}
		else {
			Debug::Log(GetFileName(__FILE__) +
				"Cannot get playerStateMachine");
		}

		// HP�Q�[�W�����ɖ߂�
		if (isValid(hpGauge)) {
			hpGauge->ChangePowerUpColor(false);
		}
		else {
			Debug::Log(GetFileName(__FILE__) +
				"Cannot get hpGauge");
		}

		// �S��ʃp���[�A�b�v�G�t�F�N�g���I����
		if (isValid(screenEffects)) {
			screenEffects->FinishPowerUpEffect();
		}
		else {
			Debug::Log(GetFileName(__FILE__) +
				"Cannot get screenEffects");
		}

		{	// �A�[�}�[������G�t�F�N�g���쐬����	
			Engine* engine = GetOwner()->GetEngine();
			const auto& p = engine->Create<GameObject>("ArmorBreak");
			{
				p->position = GetOwner()->position;
				const auto& ps = p->AddComponent<ParticleSystem>();
				ps->SpawnParticle<ArmorBreakParticle>();
			}
		}

		powerUpStep = PowerUpStep::Finish;
		break;

	case PowerUpStep::Finish:
		// �G�~�b�V�����������Ă���
		for (int i = 0; i < 4; ++i) {
			emisColor[i] = std::max(emisColor[i] - deltaSeconds, 0.0f);
		}
		// �G�~�b�V���������S�ɏ�������p���[�A�b�v���I������
		if (emisColor == 0.0f) {
			isPoweredUp = false;
		}
		break;
	}
}


void PlayerComponent::AnyDamage(
	float damage, GameObject* damageObject, const ContactPoint& contactPoint)
{
	// �C���^�[�o���𒲂ׂ�
	if (lastDamage_elapsedTime_s < damageInterval_s) {
		return;
	}
	// �X�y�V�����U���� �܂��� ����ł��� �Ȃ�_���[�W���󂯂Ȃ�
	if (playerStateMachine->GetCurrentStateType() == PlayerState::Type::Death || 
		playerStateMachine->GetCurrentStateType() == PlayerState::Type::SpecialAttack) 
	{
		return;
	}
	// �p���[�A�b�v���Ȃ�΃_���[�W���󂯂Ȃ�
	if (isPoweredUp) {
		// �I�������O�ł���΃p���[�A�b�v���I��������
		if (powerUpStep < PowerUpStep::FinishStanby) {	
			powerUpStep = PowerUpStep::FinishStanby;	// �p���[�A�b�v���I��������
			animMesh->materials[0]->emission[3] = 25.0f;
			lastDamage_elapsedTime_s = 0;

			// �A�[�}�[�̃K�[�h�����Đ�
			EasyAudio::PlaySound2D<SC_PlayerArmorGuard>();
			return;
		}
	}

	// �v���C���[�X�e�[�g�}�V���̂��Ȃ���Ώ������Ȃ�
	if (!isValid(playerStateMachine)) {
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get playerStateMachine");
		return;
	}

	// Hp�����炷
	--currentHitPoints;

	// Hp�̒l���Q�[�W�ɔ��f
	if (isValid(hpGauge)) {		
		hpGauge->SetHitPoints(currentHitPoints);
	}
	else {
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get hpGauge");
	}

	// �q�b�g�G�t�F�N�g���쐬����
	Engine* engine = GetOwner()->GetEngine();
	const auto& hitEffect = engine->Create<GameObject>("HitEffect");
	{
		hitEffect->position = contactPoint.position;
		const auto& ps = hitEffect->AddComponent<ParticleSystem>();
		ps->SpawnParticle<PlayerDamageParticle>();
	}

	// �S��ʃ_���[�W�G�t�F�N�g�����s
	if (isValid(screenEffects)) {
		screenEffects->PlayDamageEffect();
	}
	else {
		screenEffects.reset();
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get screenEffects");
	}

	// �_���[�W���ʉ����Đ�
	EasyAudio::PlaySound2D<SC_PlayerDamage>();
	
	// �_���[�W���󂯂������v�Z���u���b�N�{�[�h�ɕۑ�
	const int dc = RelativeFrontBackDirection(GetOwner(), damageObject);
	BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
	blackBoard->SetValue<int>("DamageCross", dc);

	// Hp���܂��c���Ă��邩
	if (currentHitPoints > 0) {
		// HP���܂��c���Ă�����_���[�W�X�e�[�g��		
		playerStateMachine->ChangeState<P_DamageState>();
	}
	else {
		// HP���Ȃ��Ȃ�������ꂽ�X�e�[�g��
		playerStateMachine->ChangeState<P_DeathState>();

		isTargeted = false;

		// �A�j���[�V�������b�V���̃G�~�b�V����������
		if (isValid(animMesh)) {
			animMesh->materials[0]->emission = vec4(0);
		}

		// �V�[���X�e�[�g���Q�[���I�[�o�[�ɐݒ�
		engine->GetCurrentScene()->ChangeSceneState<MG_PlayerLose>(*engine);
	}

	lastDamage_elapsedTime_s = 0;
}

void PlayerComponent::Start()
{
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	// �J�����X�^���h���擾
	if (const auto& obj = engine->FindGameObjectName("CameraStand")) {
		cameraStand = obj->GetComponent<CameraStand>();
	}
	if (isValid(cameraStand)) {
		// �J�����X�^���h�����g�̈ʒu�Ɉړ�
		cameraStand->GetOwner()->position = owner->position;	
	}
	else {
		Debug::Log(GetFileName(__FILE__) + "Cannot get cameraStand");
	}

	// �v���C���[�X�e�[�g�}�V�����擾
	playerStateMachine = owner->GetComponent<PlayerStateMachine>();
	if (isValid(playerStateMachine)) {
		// �u���b�N�{�[�h�Ƀp�����[�^��ݒ�
		BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
		blackBoard->SetValue<float>("WalkSpeed", walkSpeed);
		blackBoard->SetValue<float>("RunSpeed", runSpeed);
		blackBoard->SetValue<float>("JumpPower", jumpPower);
	}
	else {
		Debug::Log(GetFileName(__FILE__) + "Cannot get playerStateMachine");
	}

	// �A�j���[�V�������b�V�����擾
	animMesh = owner->GetComponent<GltfAnimatedMeshRenderer>();
	if (! isValid(animMesh)) {
		Debug::Log(GetFileName(__FILE__) + "Cannot get animMesh");
	}

	currentHitPoints = maxHitPoints;	//HP��ݒ�

	// �S��ʃG�t�F�N�g�I�u�W�F�N�g����R���|�[�l���g���擾
	if (const auto& obj = engine->FindGameObjectWithTag("ScreenEffects")) {
		screenEffects = obj->GetComponent<ScreenEffects>();	
	}
	if (! isValid(screenEffects)) {
		Debug::Log(GetFileName(__FILE__) + "Cannot get screenEffects");
	}

	// HP�Q�[�W�I�u�W�F�N�g����R���|�[�l���g���擾
	if (const auto& obj = engine->FindGameObjectWithTag("PlayerHpGauge")) {
		hpGauge = obj->GetComponent<PlayerHpGauge>();
	}
	if (isValid(hpGauge)) {
		hpGauge->SetMaxHitPoints(maxHitPoints);
		hpGauge->SetHitPoints(currentHitPoints);
	}
	else {
		Debug::Log(GetFileName(__FILE__) + "Cannot get hpGauge");
	}




	// �S��ʃp���[�A�b�v�G�t�F�N�g�����s
	if (isValid(screenEffects)) {
		ed_ReceiveDamage.AddEvent(
			std::bind(&ScreenEffects::StartPowerUpEffect, screenEffects));	
	}
	else {
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get screenEffects");
	}


}

void PlayerComponent::Update(float deltaSeconds)
{
	// �����h�~�[�u
	GameObject* owner = GetOwner();
	owner->position.y = std::max(owner->position.y, 0.5f);

	// �Ō�̃_���[�W����̌o�ߎ���
	lastDamage_elapsedTime_s += deltaSeconds;

	// �p���[�A�b�v�X�V����
	UpdatePowerUp(deltaSeconds);
}

void PlayerComponent::EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat)
{
	// Hp���Ȃ������珈�����Ȃ�
	if (currentHitPoints <= 0) {	
		return;
	}

	// �J�����̒����_�Ɏ��g�̍��W��ݒ�
	if (InputSystem::GetActionStateDown("Player", "Targeting")) {
		SetTargeting(!isTargeted);
	}

	// ���ڂ��Ă��邩
	if (isTargeted) {
		// ���ڂ��Ă���
		if (const auto& tObj = targetObject.lock()) {
			// ���ڒ��̃^�[�Q�b�g��������Ԃ�
			if (tObj->CheckActive()) {
				if (isValid(cameraStand)) {
					GameObject* owner = GetOwner();

					// �v���C���[�ƃh���S���́u6:4�v�̈ʒu��ݒ�
					cameraStand->targetPosition =
						owner->position + (tObj->position - owner->position) * 0.4f;

					// �v���C���[�ƃh���S���̋����ɉ����Ē����_����̃J�����̋�����ς���
					cameraStand->targetDistance =
						std::max(distance(owner->position, tObj->position) * 0.9f, 5.0f);
					
					cameraStand->trackSpeed = 5.0f;
				}
				else {
					cameraStand.reset();
					Debug::Log(GetFileName(__FILE__) + 
						"Cannot get cameraStand");
				}
			}
			else {
				targetObject.reset();

				// ���ڒ��̃^�[�Q�b�g���񊈓���ԂȂ璍�ڂ���߂�
				isTargeted = false;
			}
		}
		else {
			// ���ڒ��̃^�[�Q�b�g�����Ȃ������璍�ڂ���߂�
			isTargeted = false;
		}
	}
	else {
		// ���ڂ��Ă��Ȃ�
		if (isValid(cameraStand)) {
			cameraStand->targetPosition = vec3(ownerWorldTransMat[3]);
			cameraStand->targetDistance = 4;

			cameraStand->trackSpeed = 7.5f;
		}
		else {
			cameraStand.reset();
			Debug::Log(GetFileName(__FILE__) + "Cannot get cameraStand");
		}
	}
}

void PlayerComponent::DrawImGui()
{
	int hp = currentHitPoints;
	ImGui::SliderInt("CurrentHitPoints", &hp, 0, maxHitPoints);
	if (hp != currentHitPoints) {
		if (hpGauge) {
			hpGauge->SetHitPoints(hp);
		}
		currentHitPoints = hp;
	}
	ImGui::Spacing();

	ImGui::DragInt("HitPoints", &maxHitPoints);
	ImGui::DragFloat("DamageInterval", &damageInterval_s);
	ImGui::Spacing();

	float ws = walkSpeed;
	ImGui::DragFloat("WalkSpeed", &ws);
	float rs = runSpeed;
	ImGui::DragFloat("RunSpeed", &rs);
	float jp = jumpPower;
	ImGui::DragFloat("JumpPower", &jp);
	ImGui::Spacing();

	ImGui::DragFloat("DamageInterval", &damageInterval_s);
	ImGui::Spacing();

	ImGui::DragFloat("PowerUpTime", &powerUpDuration_s);
	if (ImGui::Button("PowerUp!!")) {
		PowerUp();
	}

	// ���l���قȂ��Ă�����X�e�[�g�}�V���̃p�����[�^���X�V
	if (playerStateMachine) {	/* �f�o�b�N�p�̏����̂Ȃߐ����m�F�͂��Ă��Ȃ� */
		BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
		if (ws != walkSpeed) {
			walkSpeed = ws;
			blackBoard->SetValue<float>("WalkSpeed", walkSpeed);
		}
		if (rs != runSpeed) {
			runSpeed = rs;
			blackBoard->SetValue<float>("RunSpeed", runSpeed);
		}
		if (jp != jumpPower) {
			jumpPower = jp;
			blackBoard->SetValue<float>("JumpPower", jumpPower);
		}
	}
	ImGui::Spacing();

	ImGui::SeparatorText("");
	if (ImGui::Button("AnyDamage")) {
		AnyDamage(1, GetOwner(), ContactPoint());
	}

}
