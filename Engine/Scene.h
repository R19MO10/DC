/**
* @file Scene.h
*/
#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED
#include "../Engine/DebugLog.h"
#include "../State/SceneState.h"
#include <memory>

// 先行宣言
class Engine;
struct Material;
using  MaterialPtr = std::shared_ptr<Material>;

/**
* シーンの基底クラス
*/
class Scene
{
public:
	SceneState_UniquePtr currentState;	// 現在のシーンステート

	MaterialPtr skysphereMaterial;	// スカイスフィア用のマテリアル

public:
	Scene() = default;
	virtual ~Scene() = default;

public:
	/**
	* シーンステートを変更する
	*
	* @tparam T ステートクラス
	*/
	template<typename SS>
	inline void ChangeSceneState(Engine& engine) {
		// 指定されたテンプレートがベースステートの派生か確認
		if constexpr (!std::is_base_of<SceneState, SS>::value) {
			LOG_WARNING("テンプレート引数がステートクラスと一致しません");
			return;
		}

		auto newState = std::make_unique<SS>();

		if (currentState) {
			currentState->Exit(*this, engine);
			currentState.reset();
		}
		if (newState) {
			currentState = std::move(newState);
			currentState->Enter(*this, engine);
		}
	}

public:
	virtual bool Initialize(
		Engine& engine) { return true; }
	virtual void Update(
		Engine& engine, float deltaSeconds) 
	{
		if (currentState) {
			currentState->Update(*this, engine, deltaSeconds);
		}
	}
	virtual void Finalize(Engine& engine) 
	{
		if (currentState) {
			currentState->Exit(*this, engine);
		}
	}

	virtual void DrawImGui() {}
};

using ScenePtr = std::shared_ptr<Scene>; // シーンポインタ型

#endif // SCENE_H_INCLUDED