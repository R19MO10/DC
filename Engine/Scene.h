/**
* @file Scene.h
*/
#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED
#include "../Engine/DebugLog.h"
#include "../State/SceneState.h"
#include <memory>

// ��s�錾
class Engine;
struct Material;
using  MaterialPtr = std::shared_ptr<Material>;

/**
* �V�[���̊��N���X
*/
class Scene
{
public:
	SceneState_UniquePtr currentState;	// ���݂̃V�[���X�e�[�g

	MaterialPtr skysphereMaterial;	// �X�J�C�X�t�B�A�p�̃}�e���A��

public:
	Scene() = default;
	virtual ~Scene() = default;

public:
	/**
	* �V�[���X�e�[�g��ύX����
	*
	* @tparam T �X�e�[�g�N���X
	*/
	template<typename SS>
	inline void ChangeSceneState(Engine& engine) {
		// �w�肳�ꂽ�e���v���[�g���x�[�X�X�e�[�g�̔h�����m�F
		if constexpr (!std::is_base_of<SceneState, SS>::value) {
			LOG_WARNING("�e���v���[�g�������X�e�[�g�N���X�ƈ�v���܂���");
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

using ScenePtr = std::shared_ptr<Scene>; // �V�[���|�C���^�^

#endif // SCENE_H_INCLUDED