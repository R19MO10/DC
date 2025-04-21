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
* パワーアップさせる
*/
void PlayerComponent::PowerUp()
{
	// アニメーションメッシュに発光エミッションを加える
	if (isValid(animMesh)) {
		// 青く発光させる
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

	// パワーアップサウンドを再生
	EasyAudio::PlaySound2D<SC_PlayerPowerUpStart>();

	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();
	
	// エフェクトを出現
	const auto& p = engine->Create<GameObject>("PowerUpParticle");
	p->SetParent(owner);
	const auto& ps = p->AddComponent<ParticleSystem>();
	ps->SpawnParticle<PowerUpParticle>();
}

/**
* Hpを回復させる
*/
void PlayerComponent::HealHp(int healHpCnt)
{
	currentHitPoints =
		std::min(currentHitPoints + healHpCnt, maxHitPoints);

	// HP値をゲージに反映
	if (isValid(hpGauge)) {
		hpGauge->SetHitPoints(currentHitPoints);
	}
	else {
		hpGauge.reset();
		Debug::Log(GetFileName(__FILE__) + "Cannot get hpGauge");
	}
}

/**
* 注目ターゲットを設定する
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

			/* 敵が増えた場合も考慮してあえてあらかじめドラゴンオブジェクトを設定していない */
			if (const auto& nearEnemy =
				owner->GetEngine()->FindGameObjectWithTag("Dragon"))
			{
				// 最も近い敵が活動状態なら設定する
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
* パワーアップ更新処理
*/
void PlayerComponent::UpdatePowerUp(float deltaSeconds)
{
	// パワーアップ中でなければ処理しない
	if (!isPoweredUp) {
		return;
	}

	// アニメーションメッシュがないためパワーアップを終了する
	if (!isValid(animMesh)) {
		animMesh.reset();

		// パワーアップを終了する
		isPoweredUp = false;
		// 全画面パワーアップエフェクトも終了する
		if (screenEffects) {
			screenEffects->FinishPowerUpEffect();
		}

		Debug::Log(GetFileName(__FILE__) +
			"Cannot get animMesh, so power-ups cannot be performed");
		return;
	}


	// 経過時間を加算
	powerUp_elapsedTime_s += deltaSeconds;

	// アニメーションメッシュへにエミッションへの参照
	vec4& emisColor = animMesh->materials[0]->emission;

	switch (powerUpStep)
	{
	case PowerUpStep::Ready:
		if (powerUp_elapsedTime_s > 0.02f) {
			// 青く発光させる
			emisColor = powerUpEmissionColor;

			// プレイヤーステートマシンのパラメータを強化する
			if (isValid(playerStateMachine)) {
				// パラメータ強化
				BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
				blackBoard->SetValue<float>("WalkSpeed", walkSpeed * 1.4f);
				blackBoard->SetValue<float>("RunSpeed", runSpeed * 1.4f);
				blackBoard->SetValue<float>("JumpPower", jumpPower * 1.3f);
			}
			else {
				Debug::Log(GetFileName(__FILE__) +
					"Cannot get playerStateMachine");
			}

			// HPゲージをパワーアップ色にする
			if (isValid(hpGauge)) {
				hpGauge->ChangePowerUpColor(true);
			}
			else {
				Debug::Log(GetFileName(__FILE__) +
					"Cannot get hpGauge");
			}

			ed_ReceiveDamage.DispatchEvent();

			//// 全画面パワーアップエフェクトを実行
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
		// エミッション強度を一瞬強くする
		emisColor.w = std::min(emisColor.w + deltaSeconds * 100, 15.0f);

		if (powerUp_elapsedTime_s > 0.13f) {
			powerUpStep = PowerUpStep::Fade;
		}
		break;

	case PowerUpStep::Fade:
		// エミッション強度を通常まで戻す
		emisColor.w = std::max(emisColor.w - deltaSeconds * 50, 1.0f);

		if (emisColor.w <= 1.0f) {
			powerUpStep = PowerUpStep::Wave;
		}
		break;

	case PowerUpStep::Wave:
		// エミッションをウェーブ状に実行
		emisColor.w = 1.0f + std::sin(powerUp_elapsedTime_s * 2);

		// 実行時間が満了したら終了処理へ
		if (powerUp_elapsedTime_s >= powerUpDuration_s) {
			// パワーアップの終了音を再生
			EasyAudio::PlaySound2D<SC_PlayerPowerUpFinish>();
			powerUpStep = PowerUpStep::FinishStanby;
		}
		break;

	case PowerUpStep::FinishStanby:
		// プレイヤーステートマシンのパラメータ元に戻す
		if (isValid(playerStateMachine)) {
			// パラメータを戻す
			BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
			blackBoard->SetValue<float>("WalkSpeed", walkSpeed);
			blackBoard->SetValue<float>("RunSpeed", runSpeed);
			blackBoard->SetValue<float>("JumpPower", jumpPower);
		}
		else {
			Debug::Log(GetFileName(__FILE__) +
				"Cannot get playerStateMachine");
		}

		// HPゲージを元に戻す
		if (isValid(hpGauge)) {
			hpGauge->ChangePowerUpColor(false);
		}
		else {
			Debug::Log(GetFileName(__FILE__) +
				"Cannot get hpGauge");
		}

		// 全画面パワーアップエフェクトを終了す
		if (isValid(screenEffects)) {
			screenEffects->FinishPowerUpEffect();
		}
		else {
			Debug::Log(GetFileName(__FILE__) +
				"Cannot get screenEffects");
		}

		{	// アーマーが壊れるエフェクトを作成する	
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
		// エミッションを消していく
		for (int i = 0; i < 4; ++i) {
			emisColor[i] = std::max(emisColor[i] - deltaSeconds, 0.0f);
		}
		// エミッションが完全に消えたらパワーアップを終了する
		if (emisColor == 0.0f) {
			isPoweredUp = false;
		}
		break;
	}
}


void PlayerComponent::AnyDamage(
	float damage, GameObject* damageObject, const ContactPoint& contactPoint)
{
	// インターバルを調べる
	if (lastDamage_elapsedTime_s < damageInterval_s) {
		return;
	}
	// スペシャル攻撃中 または 死んでいる ならダメージを受けない
	if (playerStateMachine->GetCurrentStateType() == PlayerState::Type::Death || 
		playerStateMachine->GetCurrentStateType() == PlayerState::Type::SpecialAttack) 
	{
		return;
	}
	// パワーアップ中ならばダメージを受けない
	if (isPoweredUp) {
		// 終了処理前であればパワーアップを終了させる
		if (powerUpStep < PowerUpStep::FinishStanby) {	
			powerUpStep = PowerUpStep::FinishStanby;	// パワーアップを終了させる
			animMesh->materials[0]->emission[3] = 25.0f;
			lastDamage_elapsedTime_s = 0;

			// アーマーのガード音を再生
			EasyAudio::PlaySound2D<SC_PlayerArmorGuard>();
			return;
		}
	}

	// プレイヤーステートマシンのがなけらば処理しない
	if (!isValid(playerStateMachine)) {
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get playerStateMachine");
		return;
	}

	// Hpを減らす
	--currentHitPoints;

	// Hpの値をゲージに反映
	if (isValid(hpGauge)) {		
		hpGauge->SetHitPoints(currentHitPoints);
	}
	else {
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get hpGauge");
	}

	// ヒットエフェクトを作成する
	Engine* engine = GetOwner()->GetEngine();
	const auto& hitEffect = engine->Create<GameObject>("HitEffect");
	{
		hitEffect->position = contactPoint.position;
		const auto& ps = hitEffect->AddComponent<ParticleSystem>();
		ps->SpawnParticle<PlayerDamageParticle>();
	}

	// 全画面ダメージエフェクトを実行
	if (isValid(screenEffects)) {
		screenEffects->PlayDamageEffect();
	}
	else {
		screenEffects.reset();
		Debug::Log(GetFileName(__FILE__) +
			"Cannot get screenEffects");
	}

	// ダメージ効果音を再生
	EasyAudio::PlaySound2D<SC_PlayerDamage>();
	
	// ダメージを受けた方向計算しブラックボードに保存
	const int dc = RelativeFrontBackDirection(GetOwner(), damageObject);
	BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
	blackBoard->SetValue<int>("DamageCross", dc);

	// Hpがまだ残っているか
	if (currentHitPoints > 0) {
		// HPがまだ残っていたらダメージステートへ		
		playerStateMachine->ChangeState<P_DamageState>();
	}
	else {
		// HPがなくなったらやられたステートへ
		playerStateMachine->ChangeState<P_DeathState>();

		isTargeted = false;

		// アニメーションメッシュのエミッションを消す
		if (isValid(animMesh)) {
			animMesh->materials[0]->emission = vec4(0);
		}

		// シーンステートをゲームオーバーに設定
		engine->GetCurrentScene()->ChangeSceneState<MG_PlayerLose>(*engine);
	}

	lastDamage_elapsedTime_s = 0;
}

void PlayerComponent::Start()
{
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	// カメラスタンドを取得
	if (const auto& obj = engine->FindGameObjectName("CameraStand")) {
		cameraStand = obj->GetComponent<CameraStand>();
	}
	if (isValid(cameraStand)) {
		// カメラスタンドを自身の位置に移動
		cameraStand->GetOwner()->position = owner->position;	
	}
	else {
		Debug::Log(GetFileName(__FILE__) + "Cannot get cameraStand");
	}

	// プレイヤーステートマシンを取得
	playerStateMachine = owner->GetComponent<PlayerStateMachine>();
	if (isValid(playerStateMachine)) {
		// ブラックボードにパラメータを設定
		BlackBoard* blackBoard = playerStateMachine->GetBlackboard();
		blackBoard->SetValue<float>("WalkSpeed", walkSpeed);
		blackBoard->SetValue<float>("RunSpeed", runSpeed);
		blackBoard->SetValue<float>("JumpPower", jumpPower);
	}
	else {
		Debug::Log(GetFileName(__FILE__) + "Cannot get playerStateMachine");
	}

	// アニメーションメッシュを取得
	animMesh = owner->GetComponent<GltfAnimatedMeshRenderer>();
	if (! isValid(animMesh)) {
		Debug::Log(GetFileName(__FILE__) + "Cannot get animMesh");
	}

	currentHitPoints = maxHitPoints;	//HPを設定

	// 全画面エフェクトオブジェクトからコンポーネントを取得
	if (const auto& obj = engine->FindGameObjectWithTag("ScreenEffects")) {
		screenEffects = obj->GetComponent<ScreenEffects>();	
	}
	if (! isValid(screenEffects)) {
		Debug::Log(GetFileName(__FILE__) + "Cannot get screenEffects");
	}

	// HPゲージオブジェクトからコンポーネントを取得
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




	// 全画面パワーアップエフェクトを実行
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
	// 落下防止措置
	GameObject* owner = GetOwner();
	owner->position.y = std::max(owner->position.y, 0.5f);

	// 最後のダメージからの経過時間
	lastDamage_elapsedTime_s += deltaSeconds;

	// パワーアップ更新処理
	UpdatePowerUp(deltaSeconds);
}

void PlayerComponent::EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat)
{
	// Hpがなかったら処理しない
	if (currentHitPoints <= 0) {	
		return;
	}

	// カメラの注視点に自身の座標を設定
	if (InputSystem::GetActionStateDown("Player", "Targeting")) {
		SetTargeting(!isTargeted);
	}

	// 注目しているか
	if (isTargeted) {
		// 注目している
		if (const auto& tObj = targetObject.lock()) {
			// 注目中のターゲットが活動状態か
			if (tObj->CheckActive()) {
				if (isValid(cameraStand)) {
					GameObject* owner = GetOwner();

					// プレイヤーとドラゴンの「6:4」の位置を設定
					cameraStand->targetPosition =
						owner->position + (tObj->position - owner->position) * 0.4f;

					// プレイヤーとドラゴンの距離に応じて注視点からのカメラの距離を変える
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

				// 注目中のターゲットが非活動状態なら注目をやめる
				isTargeted = false;
			}
		}
		else {
			// 注目中のターゲットがいなかったら注目をやめる
			isTargeted = false;
		}
	}
	else {
		// 注目していない
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

	// 数値が異なっていたらステートマシンのパラメータを更新
	if (playerStateMachine) {	/* デバック用の処理のなめ生存確認はしていない */
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
