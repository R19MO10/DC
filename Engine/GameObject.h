/**
* @file GameObject.h
*/
#ifndef GAMEOBJECT_H_INCLUDED
#define GAMEOBJECT_H_INCLUDED
#include "../Math/MyMath.h"
#include "DebugLog.h"
#include "Component.h"
#include <string>
#include <vector>
#include <memory>

// 先行宣言
class Engine;

class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>; // ゲームオブジェクトのポインタ
using GameObjectList = std::vector<GameObjectPtr>; // ゲームオブジェクトポインタの配列
using GameObject_WeakPtr = std::weak_ptr<GameObject>;	//ゲームオブジェクトのウィークポインタ
using GameObject_WeakList = std::vector<GameObject_WeakPtr>;	//ゲームオブジェクトウィークポインタの配列

class Renderer;
using RendererPtr = std::shared_ptr<Renderer>;
using RendererList = std::vector<RendererPtr>;
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;
using ColliderList = std::vector<ColliderPtr>;
class Rigidbody;

/**
* 描画の順序
*/
enum RenderQueue : uint8_t
{
	RenderQueue_stoneShadow,// 石の影反映図形

	RenderQueue_geometry,	// 一般的な図形
	RenderQueue_sprcular,	// スペキュラ反映図形
	RenderQueue_effect,		// エフェクト
	RenderQueue_transparent,// 半透明な図形
	RenderQueue_overlay,	// UI, 全画面エフェクトなど
};

/**
* レイヤー
*/
enum Layer
{
	Layer_Default,
	Layer_UI,
};


/**
* ゲームに登場するさまざまなオブジェクトを表す基本クラス
*/
class GameObject
{
	friend Engine;
public:
	bool isActive = true;	// オブジェクトの活動状態

	MyFName name;				// オブジェクト名
	vec3 position = vec3(0);    // 物体の位置
	vec3 rotation = vec3(0);    // 物体の回転角度
	vec3 scale = vec3(1);		// 物体の拡大率
	vec4 color = vec4(1);		// 物体の色

	int renderQueue = RenderQueue_geometry; // 描画順
	int layer = Layer_Default;		// レイヤー

private:
	Engine* engine = nullptr;       // エンジンのアドレス
	std::vector<MyFName> tags;		// タグ名

	bool isDestroyed = false;       // 死亡フラグ

	mat4 localTransMat = mat4(1);	// ローカル座標変換行列	
	mat4 worldTransMat = mat4(1);	// ワールド座標変換行列

	GameObject* parent = nullptr;		// 親オブジェクト
	std::vector<GameObject*> children;  // 子オブジェクト

	ComponentList components;	// コンポーネント配列
	RendererList renderers;		// レンダー配列
	ColliderList colliders;		// コライダー配列

	Rigidbody* rigidbody = nullptr;	// リジッドボディコンポーネント

public:
	GameObject() = default;
	virtual ~GameObject();

	// コピーと代入を禁止
	GameObject(GameObject&) = delete;
	GameObject& operator=(GameObject&) = delete;


	// タグを取得
	inline MyFName GetTag(size_t index = 0) const { 
		if (index < tags.size()) {
			return tags[index];
		}

		return MyFName();
	}
	inline std::vector<MyFName> GetTags() const {
		return tags;
	}

	// タグを追加する
	void AddTag(const char* tag);

	// タグを削除する
	void RemoveTag(size_t index);

	// タグを全て削除する
	void CleanTags();


	/**
	* 親オブジェクトの活動状態まで調べて、自身が活動状態であるか調べる
	*/
	inline bool CheckActive() const {
		if (isActive) {
			if (!parent) {
				// 親がいない場合
				return true;
			}
			return parent->CheckActive();
		}
		return false;
	}

	// ゲームエンジンを取得
	inline Engine* GetEngine() const { return engine; }

	// ゲームオブジェクトをエンジンから削除する
	inline void Destroy() { isDestroyed = true; }

	// ゲームオブジェクトが破壊されていたらtrueを返す
	inline bool IsDestroyed() const { return isDestroyed; }

	/*
	* 親オブジェクトを取得する
	*
	* @retval nullptr     親がいない
	* @retval nullptr以外 親オブジェクトのアドレス
	*/
	inline GameObject* GetParent() const { return parent; }

	/**
	* 親オブジェクトを設定する
	*
	* @param parent 親にするゲームオブジェクト
	*               nullptrを指定すると親子関係を解消する
	*/
	void SetParent(GameObject* parent);
	void SetParent(const GameObjectPtr& parent);

	/**
	* 子オブジェクトの数を取得する
	*
	* @return 子オブジェクトの数
	*/
	inline size_t GetChildCount() const { return children.size(); }

	/**
	* 子オブジェクトを取得する
	*
	* @param index 子オブジェクト配列の添字
	*
	* @return index番目の子オブジェクト
	*/
	inline GameObject* GetChild(size_t index) const {
		if (GetChildCount() <= index) {
			// 子オブジェクトの数以上の値が指定されたらnullを返す
			return nullptr;
		}
		return children[index];
	}

	/**
	* 子オブジェクトを全て取得する
	*/
	inline std::vector<GameObject*> GetChildren() const {
		return children;
	}

	// ローカル座標変換行列を取得する
	inline const mat4& GetLocalTransformMatrix() const { return localTransMat; }

	// ワールド座標変換行列を取得する
	inline const mat4& GetWorldTransformMatrix() const { return worldTransMat; }

	// コライダー配列を取得する
	inline const ColliderList& GetColliders() const { return colliders; }

	// リジッドボディコンポーネントを取得する
	inline Rigidbody* GetRigidbody() const { return rigidbody; }

	// ゲームオブジェクトにコンポーネントを追加する
	template<typename Comp>
	inline std::shared_ptr<Comp> AddComponent() {
		if constexpr (!std::is_base_of<Component, Comp>::value) {
			LOG_WARNING("コンポーネントではないテンプレートが指定されました");
			return nullptr;
		}

		const auto& p = std::make_shared<Comp>();
		if (!p) {
			LOG_WARNING("コンポーネントの追加に失敗しました");
			return nullptr;
		}
		p->owner = this;

		if constexpr (std::is_base_of_v<Renderer, Comp>) {
			renderers.push_back(p);
		}

		if constexpr (std::is_base_of_v<Collider, Comp>) {
			colliders.push_back(p);
		}
		
		if constexpr (std::is_base_of_v<Rigidbody, Comp>) {
			if (!rigidbody) {
				rigidbody = p.get();
			}
			else {
				// すでにリジッドボディコンポーネントが設定されているため
				// コンポーネントを追加しない
				LOG_WARNING("リジッドボディコンポーネントは1つしか追加できません");
				return nullptr;
			}
		}

		components.push_back(p);

		p->Awake();

		return p;
	}

	/**
	* コンポーネントを検索する
	*
	* 多くのコンポーネントを持つゲームオブジェクトに対して使用すると、
	* 検索に時間がかかる場合があるので注意。
	*
	* @tparam T 検索するコンポーネントの型
	*
	* @retval nullptr以外 T型の最初のコンポーネントへのポインタ
	* @retval nullptr     T型のコンポーネントを持っていない
	*/
	template<typename Comp>
	inline std::shared_ptr<Comp> GetComponent() {
		if constexpr (!std::is_base_of<Component, Comp>::value) {
			LOG_WARNING("コンポーネントではないテンプレートが指定されました");
			return nullptr;
		}

		for (const auto& e : components) {
			// shared_ptrの場合はdynamic_castではなくdynamic_pointer_castを使う
			const auto& p = std::dynamic_pointer_cast<Comp>(e);
			if (p) {
				return p; // 見つかったコンポーネントを返す
			}
		}
		return nullptr; // 見つからなかったのでnullptrを返す
	}

	/**
	* コンポーネントを取得する
	* 
	* @param index 要素番号
	* 
	* @retval nullptr以外 要素番号のコンポーネントへのポインタ
	* @retval nullptr	  要素番号が範囲外
	*/
	inline ComponentPtr GetComponent(const size_t& index) {
		// インデックスがコンポーネントの数以上ならnullptrを返す
		if (index < components.size()) {
			return components[index];
		}
		return nullptr;
	}

	// ゲームオブジェクトからコンポーネントを削除する
	void RemoveDestroyedComponent();

public:
	// イベント制御

	// 最初のUpdateの直前で呼び出される
	virtual void Start();

	// 毎フレーム1回呼び出される
	virtual void Update(float deltaSeconds);

	// ワールド座標計算後の処理
	virtual void EndUpdate(float deltaSeconds);

	/**
	* コライダー衝突時に呼び出される
	*
	* @param self  衝突したコライダー(自分)
	* @param other 衝突したコライダー(相手)
	* @param contactPoint 衝突情報を保持する構造体
	*/
	virtual void OnCollisionHit(
		const ColliderPtr& self, const ColliderPtr& other, const ContactPoint& contactPoint);

	// 削除されるときに呼び出される
	virtual void OnDestroy();

	// ImGuiにゲームオブジェクト情報を表示する
	virtual void DrawImGui();
};

#endif // GAMEOBJECT_H_INCLUDED