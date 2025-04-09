/**
* @file Engine.h
*/
#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <array>
#include <string>
#include <utility>
#include <algorithm>
#include "../Utility/Raycast.h"
#include "../Utility/InsidePoint.h"
#include "GameObject.h"
#include "Scene.h"
#include "../Component/Camera.h"


// ��s�錾
class FreeRoamCamera;

class ProgramPipeline;
using ProgramPipelinePtr = std::shared_ptr<ProgramPipeline>;
class FramebufferObject;
using FramebufferObjectPtr = std::shared_ptr<FramebufferObject>;

struct StaticMesh;
using StaticMeshPtr = std::shared_ptr<StaticMesh>;
struct GltfFile;
using GltfFilePtr = std::shared_ptr<GltfFile>;
struct GltfFileRange;
using AnimationMatrices = std::vector<mat4>;
class Light;
using LightPtr = std::shared_ptr<Light>;
class ParticleEmitterParameter;
class ParticleEmitter;
using ParticleEmitterPtr = std::shared_ptr<ParticleEmitter>;
class Texture;
using TexturePtr = std::shared_ptr<Texture>;

class MeshBuffer;
using MeshBufferPtr = std::shared_ptr<MeshBuffer>;
class GltfFileBuffer;
using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;
class LightBuffer;
using LightBufferPtr = std::shared_ptr<LightBuffer>;
class ParticleBuffer;
using ParticleBufferPtr = std::shared_ptr<ParticleBuffer>;

class GameObjectPrefab;

class DirectionalLight;
using DirectionalLightPtr = std::shared_ptr<DirectionalLight>;
class Collider;
class UILayout;
using UILayoutPtr = std::shared_ptr<UILayout>;
class SpriteRenderer;
using SpriteRendererPtr = std::shared_ptr<SpriteRenderer>;


/**
* �Q�[���G���W��
*/
class Engine
{
private:
	GLFWwindow* debugWindow = nullptr;				// �f�o�b�N�E�B���h�E�I�u�W�F�N�g
	const std::string debugTitle = "Debug ImGui";	// �f�o�b�N�E�B���h�E�^�C�g��

	GameObject_WeakPtr selectingGameObjct;	// GUI�őI�����Ă���Q�[���I�u�W�F�N�g
	bool showCollision = false;	// �R���W����������
	vec4 collisionColor[4] = {	// �R���W�����\���p�J���[
		{ 1.0f, 0.0f, 0.2f, 0.5f },	// �ʏ�
		{ 0.1f, 0.1f, 0.1f, 0.5f },	// ��A�N�e�B�u
		{ 1.0f, 1.0f, 0.0f, 0.5f },	// �g���K�[
		{ 0.0f, 1.0f, 0.5f, 0.5f },	// �X�^�e�B�b�N
	};

private:
	GLFWwindow* window = nullptr;				// �E�B���h�E�I�u�W�F�N�g
	const std::string title = "Dragon Circus";	// �E�B���h�E�^�C�g��

	bool isFullScreen = false;		// �t���X�N���[�����[�h�ł��邩
	bool showMouseCursor = true;	// �}�E�X�J�[�\���̕\���ݒ�

	FreeRoamCamera* freeCamera = nullptr;	// �t���[�J����

	float gameSpeed = 1.0f;	// �Q�[���������x
	float timeScale = 1.0f;	// �Q�[�����̎��Ԃ̌o�ߑ��x

private:
	int	  maxFps = 60;
	float oneFpsTime_s = 1.0f / static_cast<float>(maxFps);

	// FPS�v���p�̕ϐ�
	double fpsTime = glfwGetTime();
	double fpsCount = 0;
	double previousTime_s = 0; // �O��X�V���̎���
	float deltaSeconds = 0;     // �O��X�V����̌o�ߎ���
	float fps = 0;

private:
	ProgramPipelinePtr progWindow;	// �E�B���h�E�\���V�F�[�_

	// �I�u�W�F�N�g�`��V�F�[�_
	ProgramPipelinePtr progDefaultLit;		// �ʏ�V�F�[�_
	ProgramPipelinePtr progUnlit;			// �A�����b�g�V�F�[�_
	ProgramPipelinePtr progAnimationLit;	// �A�j���[�V�����V�F�[�_
	ProgramPipelinePtr progAnimationUnlit;	// �A�j���[�V�����A�����b�g�V�F�[�_
	ProgramPipelinePtr progSprcular;		// ���ʔ��˃V�F�[�_
	ProgramPipelinePtr progDefaultLit_with_StoneShadow;	// �΂̉e�𔽉f������ʏ�V�F�[�_

	// ���̉e�����󂯂�V�F�[�_�̊Ǘ��ԍ��z��
	std::array<GLuint, 4> affectedLightPrograms;

	// 3D���f���̉e�`��V�F�[�_
	ProgramPipelinePtr progShadow;
	ProgramPipelinePtr progShadowAnimation;

	// �G�~�b�V�����p�V�F�[�_
	ProgramPipelinePtr progEmission;
	ProgramPipelinePtr progEmissionAnimation;

	// �u���[���V�F�[�_
	ProgramPipelinePtr progHighPassFilter;
	ProgramPipelinePtr progDownSampling;
	ProgramPipelinePtr progUpSampling;

	// �A���r�G���g�I�N���[�W�����V�F�[�_
	ProgramPipelinePtr progSAORestoreDepth; // ���`�����ϊ��V�F�[�_
	ProgramPipelinePtr progSAODownSampling; // �k���V�F�[�_
	ProgramPipelinePtr progSAOCalc;         // SAO�v�Z�V�F�[�_
	ProgramPipelinePtr progSAOBlur;         // SAO�p�ڂ����V�F�[�_
	ProgramPipelinePtr progSAO;				// SAO�K���V�F�[�_

	FramebufferObjectPtr fboMainGameWindow; // �Q�[���E�B���h�E�pFBO
	FramebufferObjectPtr fboShadow;			// �f�v�X�V���h�E�pFBO
	FramebufferObjectPtr fboEmission;		// �G�~�b�V�����pFBO
	std::array<FramebufferObjectPtr, 6> fboBloom;	// �u���[���̏k���o�b�t�@
	std::array<FramebufferObjectPtr, 4> fboSAODepth;// �[�x�l�̏k���o�b�t�@
	FramebufferObjectPtr fboSAOCalc;    // SAO�v�Z���ʃo�b�t�@
	FramebufferObjectPtr fboSAOBlur;    // SAO�ڂ������ʃo�b�t�@

	// �`�悷��GBuffer
	enum GBuffer : uint8_t {
		MainGame,
		Depth,
		Shadow,
		Emission,
		SAO,
	};
	GBuffer renderGBuffer = MainGame;

	// �u���[������p�p�����[�^
	struct BloomParam {
		bool isEnabled = true;	// �L��
		float threshold = 1;	// ���P�x�Ƃ݂Ȃ����邳(0.0�`����Ȃ�)
		float strength = 8;		// �u���[���̋���
	};
	BloomParam screenBloomParam = { true, 1.1f, 4.0f };		// ��ʂɊ|����u���[��
	BloomParam emissionBloomParam = { true, 1.0f, 8.0f };	// �G�~�b�V�����Ɋ|����u���[��

	// SAO����p�p�����[�^
	struct SAOParam {
		bool isEnabled = true;	// �L��
		float radius = 1.0f;	// ���[���h���W�n�ɂ�����AO�̃T���v�����O���a(�P��=m)
		float bias = 0.012f;	// ���ʂƂ݂Ȃ��Ė�������p�x�̃R�T�C��
		float intensity = 0.7f; // AO���ʂ̋���
	};
	SAOParam saoParam;
	
	// �o�b�t�@�I�u�W�F�N�g
	MeshBufferPtr meshBuffer;			// �I�u�W�F���b�V��
	GltfFileBufferPtr gltfFileBuffer;	// gltf���b�V��
	LightBufferPtr lightBuffer;			// ���C�g
	ParticleBufferPtr particleBuffer;	// �p�[�e�B�N��


	// �G���W���g�p���b�V��
	StaticMeshPtr windowMesh;	// �E�B���h�E�\���p���b�V��
	StaticMeshPtr skySphere;	// �X�J�C�X�t�B�A�p���b�V��

	// ����
	vec3 ambientLight = { 0.5f, 0.5f, 0.5f };
	// ���s����
	DirectionalLightPtr directionalLight;	// ���s�����R���|�[�l���g�|�C���^


private:
	ScenePtr scene;     // ���s���̃V�[��
	ScenePtr nextScene; // ���̃V�[��

private:
	GameObjectPtr mainCamera;	// ���ݎg�p���Ă���J�����u�W�F�N�g

	// �Q�[���I�u�W�F�N�g�z��
	GameObjectList gameObjects;
	// Tag�����Q�[���I�u�W�F�N�g�z��
	std::unordered_map<MyFName, GameObjectList> gameObjects_tag;

public:
	Engine() = default;
	~Engine() = default;
	int Run();

public:	// ����
	// TimeScale���l������Ă��Ȃ��o�ߎ��Ԃ��擾����
	inline float GetUnscaledDeltaSeconds() {
		return deltaSeconds * gameSpeed;
	}

	// TimeScale���擾����
	inline float GetTimeScale() { return timeScale; }
	// TimeScale��ݒ肷��
	inline void SetTimeScale(const float& scale) { timeScale = scale; }

	// �R���C�_�[�̏󋵂ɉ������F���擾����
	vec4 GetCollisionColor(const Collider& collider);

public: // ����
	// �������擾����
	inline vec3 GetAmbientLight() {
		return ambientLight;
	}
	// ������ݒ肷��
	inline void SetAmbientLight(const vec3& color) {
		ambientLight = color;
	}
	
public:	// ���s����
	// ���s�������擾����
	inline DirectionalLightPtr GetDirectionalLight() {
		return directionalLight;
	}

	// ���s������ݒ肷��
	inline void SetDirectionalLight(const DirectionalLightPtr& direclightPtr) {
		directionalLight = direclightPtr;
	}

public:	// �X�J�C�}�e���A��
	// ���݃V�[���̃X�J�C�}�e���A�����擾����
	inline MaterialPtr GetSkyMaterial() {
		return scene->skysphereMaterial;
	}
	// ���݃V�[���̃X�J�C�}�e���A����ݒ肷��
	inline void SetSkyMaterial(const MaterialPtr& materialPtr) {
		scene->skysphereMaterial = materialPtr;
	}

public:	// �Q�[���I�u�W�F�N�g
	/**
	* �Q�[���I�u�W�F�N�g���쐬����
	*
	* @param name     �I�u�W�F�N�g�̖��O
	*/
	template<typename GObj_or_Prefab>
	inline GameObjectPtr Create(const std::string& name)
	{
		if constexpr (std::is_base_of<GameObject, GObj_or_Prefab>::value)
		{
			GameObjectPtr p = std::make_shared<GObj_or_Prefab>();
			if (!p) {
				LOG_WARNING("�Q�[���I�u�W�F�N�g�̍쐬�Ɏ��s���܂���");
				return nullptr;
			}

			p->engine = this;
			p->name = name;

			gameObjects.push_back(p); // �G���W���ɓo�^

			return p;
		}
		else if constexpr (std::is_base_of<GameObjectPrefab, GObj_or_Prefab>::value)
		{
			GObj_or_Prefab prefab;
			return prefab.Create(name, *this);
		}

		LOG_WARNING("�Q�[���I�u�W�F�N�g�ł͂Ȃ��e���v���[�g���w�肳��܂���");
		return nullptr;
	}

	/**
	* UI�I�u�W�F�N�g�̍쐬�f�[�^
	*/
	struct UIObject {
		GameObjectPtr object;
		SpriteRendererPtr spriteRender;
		UILayoutPtr uiLayot;
	};
	/**
	* UI�I�u�W�F�N�g���쐬����
	*
	* @param name		UI�I�u�W�F�N�g�̖��O
	* @param filename	UI�I�u�W�F�N�g�ɕ\������摜
	* @param magnification    UI�I�u�W�F�N�g�̑傫��(�摜�T�C�Y�ɓ��{�ɑ傫���Ȃ�)
	*
	* @return �쐬����UI�I�u�W�F�N�g
	*/
	UIObject CreateUIObject(
		const std::string& name, const char* filename,
		const float& magnification = 1);

	/**
	* �w�肳�ꂽ���O�̃Q�[���I�u�W�F�N�g��T��
	* 
	* @param name ���O
	* 
	* @return �ŏ��Ɍ��������Q�[���I�u�W�F�N�g
	*/
	inline GameObjectPtr FindGameObjectName(const char* name)
	{
		MyFName fname(name);
		auto itr = std::find_if(gameObjects.begin(), gameObjects.end(),
						[&fname](const GameObjectPtr gObj) { return gObj->name == fname; });

		if (itr == gameObjects.end()) {
			// �w�肳�ꂽ���O�̃Q�[���I�u�W�F�N�g���Ȃ�����
			return nullptr;
		}

		return *itr;
	}

	/**
	* �w�肳�ꂽ�^�O�̃Q�[���I�u�W�F�N�g��T��
	*
	* @param tag �^�O��
	*
	* @return �ŏ��Ɍ��������Q�[���I�u�W�F�N�g
	*/
	inline GameObjectPtr FindGameObjectWithTag(const char* tag)
	{
		MyFName ftag(tag);
		if (gameObjects_tag.end() == gameObjects_tag.find(ftag)) {
			// �w�肳�ꂽ�^�O���^�O�����Q�[���I�u�W�F�N�g�z��ɂȂ�
			return nullptr;
		}

		const auto& objs = gameObjects_tag.at(ftag);
		if (objs.empty()) {
			// �^�O���͂��邪�Q�[���I�u�W�F�N�g���Ȃ�
			return nullptr;
		}

		return objs[0];
	}

	/**
	* �w�肳�ꂽ�^�O�̃Q�[���I�u�W�F�N�g�B��T��
	*
	* @param tag �^�O��
	*
	* @return ���������Q�[���I�u�W�F�N�g�z��
	*/
	inline GameObjectList FindGameObjectsWithTag(const char* tag)
	{
		MyFName ftag(tag);
		if (gameObjects_tag.end() == gameObjects_tag.find(ftag)) {
			// �w�肳�ꂽ�^�O���^�O�����Q�[���I�u�W�F�N�g�z��ɂȂ�
			return GameObjectList();	 // ������Ȃ������̂ŋ��GameObjectList��Ԃ�
		}

		return gameObjects_tag.at(ftag);
	}


	/**
	* �^�O�����Q�[���I�u�W�F�N�g�z��ɒǉ�����
	*
	* @param tagName	�V�����ݒ肷��^�O
	* @param gameObj	�Q�[���I�u�W�F�N�g�|�C���^
	* 
	* @return �ǉ��ɐ���������
	*/
	inline bool GameObjectAddTag(
		const MyFName& tagName, const GameObject* gameObj)
	{
		// �Q�[���I�u�W�F�N�g�z�񂩂玩�g�̃V�F�A�[�h�|�C���^��T��
		const auto& itr = std::find_if(gameObjects.begin(), gameObjects.end(),
			[&gameObj](const GameObjectPtr& gObj) {
				return gObj.get() == gameObj; });

		// ������Ȃ������珈�����Ȃ�
		if (itr == gameObjects.end()) {
			return false;
		}

		// �^�O�����Q�[���z��ɒǉ�
		gameObjects_tag[tagName].push_back(*itr);
		return true;
	}

	/**
	* �^�O�����Q�[���I�u�W�F�N�g�z�񂩂�w��^�O�̃I�u�W�F�N�g���폜����
	*
	* @param tagName	�폜����I�u�W�F�N�g�̎��^�O
	* @param gameObj	�Q�[���I�u�W�F�N�g�z��̃|�C���^
	*
	* @return �폜�ɐ���������
	*/
	inline bool GameObjectRemoveTag(
		const MyFName& tagName, const GameObject* gameObj)
	{
		const auto& objs = gameObjects_tag.find(tagName);
		if (objs == gameObjects_tag.end()) {
			// �^�O�����݂��Ȃ���Δ�΂�
			return false;
		}

		// �^�O�Q�[���I�u�W�F�N�g�z��̒��ɂ��鎩�g��T��
		const auto& itr = 
			std::find_if(objs->second.begin(), objs->second.end(),
			[&gameObj](const GameObjectPtr& gObj) {
				return gObj.get() == gameObj; });

		// ������Ȃ������珈�����Ȃ�
		if (itr == gameObjects.end()) {
			return false;
		}

		// �^�O�Q�[���I�u�W�F�N�g�z�񂩂�폜
		objs->second.erase(itr);
		return true;
	}

	// ���ׂẴQ�[���I�u�W�F�N�g���폜����
	inline void ClearGameObjectAll() {
		for (const auto& e : gameObjects) {
			e->OnDestroy();
		}
		gameObjects.clear();
		gameObjects_tag.clear();

		mainCamera.reset();
		directionalLight.reset();
	}

public:	// �S�Q�[���I�u�W�F�N�g�Ƃ̓����蔻��
	/**
	* �����Ƃ��ׂẴQ�[���I�u�W�F�N�g�̃R���C�_�[�Ƃ̌�������
	*
	* @param ray            ����
	* @param hitInfo        �����ƍŏ��Ɍ��������R���C�_�[�̏��
	* @param pred           ����������s���R���C�_�[��I�ʂ���q��
	*
	* @return �����ꂩ�̃R���C�_�[�ƌ���������
	*/
	inline bool Raycast_GameObjectAll(
		const Ray& ray, RayHitInfo& hitInfo, const RaycastPredicate& pred) 
	{
		return Raycast(ray, gameObjects, hitInfo, pred);
	}

	/**
	*  �_�Ƃ��ׂẴQ�[���I�u�W�F�N�g�̃R���C�_�[�Ƃ̐ڐG����
	*
	* @param point          �_�̍��W
	* @param pred           ����������s���R���C�_�[��I�ʂ���q��
	*
	* @return �_�ƐڐG���Ă���R���C�_�[�|�C���^�z��
	*/
	inline ColliderList InsidePoint_GameObjectAll(
		const vec3 point, const InsidePointPredicate& pred) 
	{
		return InsidePoint(point, gameObjects, pred);
	}

public:	// �V�[��
	// ���݂̃V�[�����擾����
	inline Scene* GetCurrentScene() { return scene.get(); }

	// ���̃V�[����ݒ肷��
	template<typename S>
	inline void SetNextScene() { nextScene = std::make_shared<S>(); }

public:	// �E�B���h�E
	// �E�B���h�E���t�H�[�J�X����Ă��邩
	inline bool FocusWindow() const {
		return glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
	}

public:	// �}�E�X�J�[�\��
	// �}�E�X�J�[�\���̕\����Ԃ��擾
	inline bool const GetShowMouseCursor() { return showMouseCursor; }

	// �}�E�X�J�[�\���̔�\����ݒ�
	inline void SetShowMouseCursor(const bool& isVisible) {
		showMouseCursor = isVisible;

		// �f�o�b�O�E�B���h�E���\������Ă��Ȃ������瑦���K������
		if (!debugWindow) {
			glfwSetInputMode(window, GLFW_CURSOR,
				isVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		}
	}

public:	// ���C���J����
	// ���C���J�������擾����
	inline GameObject* GetMainCameraObject() {
		if (mainCamera) {
			// ���C���J�������o�^����Ă���
			return mainCamera.get();
		}
		return nullptr;
	}
	inline const GameObject* GetMainCameraObject() const {
		if (mainCamera) {
			// ���C���J�������o�^����Ă���
			return mainCamera.get();
		}
		return nullptr;
	}

	// ���C���J������ݒ肷��
	inline void SetMainCameraObject(const GameObjectPtr& cameraObject) {
		// �J�����R���|�[�l���g�̂���z��v�f�ԍ����擾
		size_t cameraCompIndex = 0;
		for (const auto& e : cameraObject->components) {
			if (std::dynamic_pointer_cast<Camera>(e)) {
				break;
			}
			++cameraCompIndex;
		}

		if (cameraCompIndex == cameraObject->components.size()) {
			// �J�����R���|�[�l���g��������Ȃ�����
			LOG_WARNING("�J�����R���|�[�l���g���ݒ肳��Ă��܂���");
			return;
		}

		// �J�����R���|�[�l���g���R���|�[�l���g�z��̍Ō���ł͂Ȃ�������Ō���ֈړ�������
		if (cameraCompIndex != cameraObject->components.size() - 1) {
			std::swap(
				cameraObject->components[cameraCompIndex], cameraObject->components.back());
		}

		mainCamera = cameraObject;
	}

	// ���C���J�����̃J�����R���|�[�l���g���擾����
	inline Camera* GetMainCamera_CameraComponent() const {
		if (const auto& camera = GetMainCameraObject()) {
			// �R���|�[�l���g�̈�Ԍ��ɂ���J�����R���|�[�l���g���擾
			return dynamic_cast<Camera*>(camera->components.back().get());
		}
		return nullptr;
	}

public:	// �t���[���o�b�t�@
	// �t���[���o�b�t�@�̑傫�����擾����
	vec2Int GetFlamebufferSize() const;

	// �t���[���o�b�t�@�̃A�X�y�N�g����擾����
	float GetAspectRatio() const;

public:	// ���b�V���o�b�t�@
	/**
	* �X�^�e�B�b�N���b�V���̎擾
	*
	* @param name ���b�V����
	*
	* @return ���O��name�ƈ�v����X�^�e�B�b�N���b�V��
	*/
	StaticMeshPtr GetStaticMesh(const char* name);

public:	//Gltf�o�b�t�@
	/**
	* glTF�t�@�C���̎擾
	*
	* @param name glTf�t�@�C���̖��O
	*
	* @return ���O��name�ƈ�v����glTf�t�@�C��
	*/
	GltfFilePtr GetGltfFile(const char* name);

	/**
	* �A�j���[�V�������b�V���̕`��p�f�[�^��ǉ�
	*
	* @param matBones SSBO�ɒǉ�����p���s��̔z��
	+*
	* @return matBones�p�Ɋ��蓖�Ă�ꂽSSBO�͈̔�
	*/
	GltfFileRange AddAnimationMatrices(const AnimationMatrices& matBones);

	/**
	* �A�j���[�V�������b�V���̕`��Ɏg��SSBO�̈�����蓖�Ă�
	*
	* @param bindingPoint �o�C���f�B���O�|�C���g
	* @param range        �o�C���h����͈�
	*/
	void BindAnimationBuffer(GLuint bindingPoint, const GltfFileRange& range);
	
public:	// ���C�g�o�b�t�@
	/**
	* �V�������C�g���擾����
	*
	* @param lightPtr ���C�g�R���|�[�l���g�|�C���^
	*/
	void AllocateLight(const LightPtr& lightPtr);

public:	// �p�[�e�B�N���o�b�t�@
	/**
	* �G�~�b�^�[��ǉ�����
	*
	* @param  emitterParam  �G�~�b�^�[�̏������p�����[�^
	*
	* @return �ǉ������G�~�b�^�[
	*/
	ParticleEmitterPtr AddParticleEmitter(const ParticleEmitterParameter& emitterParam);

	/**
	* �w�肳�ꂽ���O�����G�~�b�^�[����������
	*
	* @param name  ��������ID
	*
	* @return ����id�ƈ�v����ID�����G�~�b�^�[
	*/
	ParticleEmitterPtr FindParticleEmitter(const char* name) const;

	/**
	* �w�肳�ꂽ�G�~�b�^�[���폜����
	*/
	void RemoveParticleEmitter(const ParticleEmitterPtr& emitterPtr);

	/**
	* ���ׂẴG�~�b�^�[���폜����
	*/
	void RemoveParticleEmitterAll();

private:
	// �t���[���o�b�t�@�Ɠ����T�C�Y��FBO���쐬����
	void MatchFboSize();
	// �J���[�e�N�X�`����FBO�ɕ`�悷��
	void DrawTextureToFbo(
		FramebufferObject& fbo, ProgramPipeline& prog, 
		TexturePtr texture, GLenum wrapMode);

private:
	int Initialize();

	void Update();
	void UpdateGameObject(float deltaSeconds);
	void CalcGameObjectTransMat(const size_t& calcStartIndex);
	void RemoveDestroyedGameObject();

	void Render();
	void CreateShadowMap(
		GameObjectList::iterator begin, GameObjectList::iterator end);
	void DrawGameObject(
		ProgramPipeline& progLit, ProgramPipeline& progUnlit, float alphaCutoff,
		GameObjectList::iterator begin, GameObjectList::iterator end);
	void DrawSkySphere(
		ProgramPipeline& prog);
	void DrawEmissionGameObject(
		float alphaCutoff,
		GameObjectList::iterator begin, GameObjectList::iterator end);

	void DrawBloomEffect(
		FramebufferObject& fbo, const BloomParam& param);
	void DrawAmbientOcclusion();

	void ShowCollision(ProgramPipeline& prog);
	void RenderImGui();
};

#endif // ENGINE_H_INCLUDED