/**
* @file Component.h
*/
#ifndef COMOPNENT_H_INCLUDED
#define COMOPNENT_H_INCLUDED
#include "../Math/MyMath.h"
#include <imgui.h>
#include <string>
#include <vector>
#include <memory>

// 先行宣言
struct ContactPoint;
class Engine;
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;
class Component;
using ComponentPtr = std::shared_ptr<Component>; // コンポーネントのポインタ
using ComponentList = std::vector<ComponentPtr>; // コンポーネントポインタ配列

/**
* コンポーネントの基底クラス
*/
class Component
{
	friend Engine;
	friend GameObject;
public:
	std::string name;	// コンポーネントの名前(ImGui表示にしか使用しないためstring型を使用)
	bool isActive = true;	// コンポーネントの活動状態

private:
	GameObject* owner = nullptr; // このコンポーネントの所有者
	bool isStarted = false;      // Startが実行されたらtrueになる
	bool isDestroyed = false;    // Destroyが実行されたらtrueになる

public:
	Component(const char* compName = "Defult Component")
		:name(compName)
	{};
	virtual ~Component() = default;

	// コンポーネントの所有者を取得
	inline GameObject* GetOwner() const { return owner; }

	// コンポーネントをゲームオブジェクトから削除する
	inline void Destroy() { isDestroyed = true; }

	// コンポーネントが破壊されていたらtrueを返す（破棄状態を示す）
	inline bool IsDestroyed() const { return isDestroyed; }

public:
	// ゲームオブジェクトに追加された時に呼び出される
	virtual void Awake() {}

	// 最初のUpdateの直前で呼び出される
	virtual void Start() {}

	// 毎フレーム1回呼び出される
	virtual void Update(float deltaSeconds) {}
	
	// ワールド座標計算後の処理
	virtual void EndUpdate(float deltaSeconds, mat4& ownerWorldTransMat) {}

	// 衝突が起きたときに呼び出される
	virtual void OnCollisionHit(
		const ColliderPtr& self, const ColliderPtr& other, const ContactPoint& contactPoint) {}

	// 削除されるときに呼び出される
	virtual void OnDestroy() {}

	// ImGuiにコンポーネント情報を出力
	virtual void DrawImGui() {}
};

#endif // COMOPNENT_H_INCLUDED