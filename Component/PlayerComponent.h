/**
* @file PlayerComponent.h
*/
#ifndef PLAYER_COMPONENT_H_INCLUDED
#define PLAYER_COMPONENT_H_INCLUDED
#include "CharacterComponent.h"
#include "../Math/MyMath.h"
#include "../Utility/EventDispatcher.h"


// 先行宣言
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
* プレイヤーのパラメータやダメージを管理
*/
class PlayerComponent : public CharacterComponent
{
private:
	CameraStandPtr cameraStand;	// カメラスタンド
	PlayerStateMachinePtr playerStateMachine;	// プレイヤーステートマシン
	GltfAnimatedMeshRendererPtr animMesh;		// アニメーションメッシュ

	ScreenEffectsPtr screenEffects;	// 全画面エフェクト
	PlayerHpGaugePtr hpGauge;		// Hpゲージ

	//プレイヤーが注目しているゲームオブジェクト（WeakPtrなのは相互参照の可能性があるため）
	GameObject_WeakPtr targetObject;	

private:
	int currentHitPoints = 0;

	bool isTargeted = false;	// ゲームオブジェクトに注目しているか

	bool isPoweredUp = false;			// パワーアップしているか
	float powerUp_elapsedTime_s = 0;	// パワーアップ経過時間
	enum PowerUpStep : uint8_t {
		Ready,	// 開始
		Flash,	// 発光を一瞬強くする
		Fade,	// 発光を通常まで戻す
		Wave,	// 発光をウェーブ間隔に行う
		FinishStanby,	// 終了準備
		Finish,	// 終了
	};
	PowerUpStep powerUpStep = Ready;	// パワーアップ処理のステップ

public:
	int maxHitPoints = 4;
	float walkSpeed = 3.0f;
	float runSpeed = 5.0f;
	float jumpPower = 5.5f;
	float damageInterval_s = 1.0f;

	float powerUpDuration_s = 10.0f;	// パワーアップの持続時間
	vec4 powerUpEmissionColor =			// パワーアップエミッショ色s
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
	// パワーアップさせる
	void PowerUp();		

	// Hpを回復させる
	void HealHp(int healHpCnt = 1);

	// 注目設定
	void SetTargeting(bool isTargeted, const GameObjectPtr& targetObj = nullptr);

private:
	// パワーアップ更新処理
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