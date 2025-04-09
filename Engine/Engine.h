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


// 先行宣言
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
* ゲームエンジン
*/
class Engine
{
private:
	GLFWwindow* debugWindow = nullptr;				// デバックウィンドウオブジェクト
	const std::string debugTitle = "Debug ImGui";	// デバックウィンドウタイトル

	GameObject_WeakPtr selectingGameObjct;	// GUIで選択しているゲームオブジェクト
	bool showCollision = false;	// コリジョンを可視化
	vec4 collisionColor[4] = {	// コリジョン表示用カラー
		{ 1.0f, 0.0f, 0.2f, 0.5f },	// 通常
		{ 0.1f, 0.1f, 0.1f, 0.5f },	// 非アクティブ
		{ 1.0f, 1.0f, 0.0f, 0.5f },	// トリガー
		{ 0.0f, 1.0f, 0.5f, 0.5f },	// スタティック
	};

private:
	GLFWwindow* window = nullptr;				// ウィンドウオブジェクト
	const std::string title = "Dragon Circus";	// ウィンドウタイトル

	bool isFullScreen = false;		// フルスクリーンモードであるか
	bool showMouseCursor = true;	// マウスカーソルの表示設定

	FreeRoamCamera* freeCamera = nullptr;	// フリーカメラ

	float gameSpeed = 1.0f;	// ゲーム処理速度
	float timeScale = 1.0f;	// ゲーム内の時間の経過速度

private:
	int	  maxFps = 60;
	float oneFpsTime_s = 1.0f / static_cast<float>(maxFps);

	// FPS計測用の変数
	double fpsTime = glfwGetTime();
	double fpsCount = 0;
	double previousTime_s = 0; // 前回更新時の時刻
	float deltaSeconds = 0;     // 前回更新からの経過時間
	float fps = 0;

private:
	ProgramPipelinePtr progWindow;	// ウィンドウ表示シェーダ

	// オブジェクト描画シェーダ
	ProgramPipelinePtr progDefaultLit;		// 通常シェーダ
	ProgramPipelinePtr progUnlit;			// アンリットシェーダ
	ProgramPipelinePtr progAnimationLit;	// アニメーションシェーダ
	ProgramPipelinePtr progAnimationUnlit;	// アニメーションアンリットシェーダ
	ProgramPipelinePtr progSprcular;		// 鏡面反射シェーダ
	ProgramPipelinePtr progDefaultLit_with_StoneShadow;	// 石の影を反映させる通常シェーダ

	// 光の影響を受けるシェーダの管理番号配列
	std::array<GLuint, 4> affectedLightPrograms;

	// 3Dモデルの影描画シェーダ
	ProgramPipelinePtr progShadow;
	ProgramPipelinePtr progShadowAnimation;

	// エミッション用シェーダ
	ProgramPipelinePtr progEmission;
	ProgramPipelinePtr progEmissionAnimation;

	// ブルームシェーダ
	ProgramPipelinePtr progHighPassFilter;
	ProgramPipelinePtr progDownSampling;
	ProgramPipelinePtr progUpSampling;

	// アンビエントオクルージョンシェーダ
	ProgramPipelinePtr progSAORestoreDepth; // 線形距離変換シェーダ
	ProgramPipelinePtr progSAODownSampling; // 縮小シェーダ
	ProgramPipelinePtr progSAOCalc;         // SAO計算シェーダ
	ProgramPipelinePtr progSAOBlur;         // SAO用ぼかしシェーダ
	ProgramPipelinePtr progSAO;				// SAO適応シェーダ

	FramebufferObjectPtr fboMainGameWindow; // ゲームウィンドウ用FBO
	FramebufferObjectPtr fboShadow;			// デプスシャドウ用FBO
	FramebufferObjectPtr fboEmission;		// エミッション用FBO
	std::array<FramebufferObjectPtr, 6> fboBloom;	// ブルームの縮小バッファ
	std::array<FramebufferObjectPtr, 4> fboSAODepth;// 深度値の縮小バッファ
	FramebufferObjectPtr fboSAOCalc;    // SAO計算結果バッファ
	FramebufferObjectPtr fboSAOBlur;    // SAOぼかし結果バッファ

	// 描画するGBuffer
	enum GBuffer : uint8_t {
		MainGame,
		Depth,
		Shadow,
		Emission,
		SAO,
	};
	GBuffer renderGBuffer = MainGame;

	// ブルーム制御用パラメータ
	struct BloomParam {
		bool isEnabled = true;	// 有無
		float threshold = 1;	// 高輝度とみなす明るさ(0.0～上限なし)
		float strength = 8;		// ブルームの強さ
	};
	BloomParam screenBloomParam = { true, 1.1f, 4.0f };		// 画面に掛けるブルーム
	BloomParam emissionBloomParam = { true, 1.0f, 8.0f };	// エミッションに掛けるブルーム

	// SAO制御用パラメータ
	struct SAOParam {
		bool isEnabled = true;	// 有無
		float radius = 1.0f;	// ワールド座標系におけるAOのサンプリング半径(単位=m)
		float bias = 0.012f;	// 平面とみなして無視する角度のコサイン
		float intensity = 0.7f; // AO効果の強さ
	};
	SAOParam saoParam;
	
	// バッファオブジェクト
	MeshBufferPtr meshBuffer;			// オブジェメッシュ
	GltfFileBufferPtr gltfFileBuffer;	// gltfメッシュ
	LightBufferPtr lightBuffer;			// ライト
	ParticleBufferPtr particleBuffer;	// パーティクル


	// エンジン使用メッシュ
	StaticMeshPtr windowMesh;	// ウィンドウ表示用メッシュ
	StaticMeshPtr skySphere;	// スカイスフィア用メッシュ

	// 環境光
	vec3 ambientLight = { 0.5f, 0.5f, 0.5f };
	// 平行光源
	DirectionalLightPtr directionalLight;	// 平行光源コンポーネントポインタ


private:
	ScenePtr scene;     // 実行中のシーン
	ScenePtr nextScene; // 次のシーン

private:
	GameObjectPtr mainCamera;	// 現在使用しているカメラブジェクト

	// ゲームオブジェクト配列
	GameObjectList gameObjects;
	// Tag持ちゲームオブジェクト配列
	std::unordered_map<MyFName, GameObjectList> gameObjects_tag;

public:
	Engine() = default;
	~Engine() = default;
	int Run();

public:	// 時間
	// TimeScaleが考慮されていない経過時間を取得する
	inline float GetUnscaledDeltaSeconds() {
		return deltaSeconds * gameSpeed;
	}

	// TimeScaleを取得する
	inline float GetTimeScale() { return timeScale; }
	// TimeScaleを設定する
	inline void SetTimeScale(const float& scale) { timeScale = scale; }

	// コライダーの状況に応じた色を取得する
	vec4 GetCollisionColor(const Collider& collider);

public: // 環境光
	// 環境光を取得する
	inline vec3 GetAmbientLight() {
		return ambientLight;
	}
	// 環境光を設定する
	inline void SetAmbientLight(const vec3& color) {
		ambientLight = color;
	}
	
public:	// 平行光源
	// 平行光源を取得する
	inline DirectionalLightPtr GetDirectionalLight() {
		return directionalLight;
	}

	// 平行光源を設定する
	inline void SetDirectionalLight(const DirectionalLightPtr& direclightPtr) {
		directionalLight = direclightPtr;
	}

public:	// スカイマテリアル
	// 現在シーンのスカイマテリアルを取得する
	inline MaterialPtr GetSkyMaterial() {
		return scene->skysphereMaterial;
	}
	// 現在シーンのスカイマテリアルを設定する
	inline void SetSkyMaterial(const MaterialPtr& materialPtr) {
		scene->skysphereMaterial = materialPtr;
	}

public:	// ゲームオブジェクト
	/**
	* ゲームオブジェクトを作成する
	*
	* @param name     オブジェクトの名前
	*/
	template<typename GObj_or_Prefab>
	inline GameObjectPtr Create(const std::string& name)
	{
		if constexpr (std::is_base_of<GameObject, GObj_or_Prefab>::value)
		{
			GameObjectPtr p = std::make_shared<GObj_or_Prefab>();
			if (!p) {
				LOG_WARNING("ゲームオブジェクトの作成に失敗しました");
				return nullptr;
			}

			p->engine = this;
			p->name = name;

			gameObjects.push_back(p); // エンジンに登録

			return p;
		}
		else if constexpr (std::is_base_of<GameObjectPrefab, GObj_or_Prefab>::value)
		{
			GObj_or_Prefab prefab;
			return prefab.Create(name, *this);
		}

		LOG_WARNING("ゲームオブジェクトではないテンプレートが指定されました");
		return nullptr;
	}

	/**
	* UIオブジェクトの作成データ
	*/
	struct UIObject {
		GameObjectPtr object;
		SpriteRendererPtr spriteRender;
		UILayoutPtr uiLayot;
	};
	/**
	* UIオブジェクトを作成する
	*
	* @param name		UIオブジェクトの名前
	* @param filename	UIオブジェクトに表示する画像
	* @param magnification    UIオブジェクトの大きさ(画像サイズに等倍に大きくなる)
	*
	* @return 作成したUIオブジェクト
	*/
	UIObject CreateUIObject(
		const std::string& name, const char* filename,
		const float& magnification = 1);

	/**
	* 指定された名前のゲームオブジェクトを探す
	* 
	* @param name 名前
	* 
	* @return 最初に見つかったゲームオブジェクト
	*/
	inline GameObjectPtr FindGameObjectName(const char* name)
	{
		MyFName fname(name);
		auto itr = std::find_if(gameObjects.begin(), gameObjects.end(),
						[&fname](const GameObjectPtr gObj) { return gObj->name == fname; });

		if (itr == gameObjects.end()) {
			// 指定された名前のゲームオブジェクトがなかった
			return nullptr;
		}

		return *itr;
	}

	/**
	* 指定されたタグのゲームオブジェクトを探す
	*
	* @param tag タグ名
	*
	* @return 最初に見つかったゲームオブジェクト
	*/
	inline GameObjectPtr FindGameObjectWithTag(const char* tag)
	{
		MyFName ftag(tag);
		if (gameObjects_tag.end() == gameObjects_tag.find(ftag)) {
			// 指定されたタグがタグ持ちゲームオブジェクト配列にない
			return nullptr;
		}

		const auto& objs = gameObjects_tag.at(ftag);
		if (objs.empty()) {
			// タグ名はあるがゲームオブジェクトがない
			return nullptr;
		}

		return objs[0];
	}

	/**
	* 指定されたタグのゲームオブジェクト達を探す
	*
	* @param tag タグ名
	*
	* @return 見つかったゲームオブジェクト配列
	*/
	inline GameObjectList FindGameObjectsWithTag(const char* tag)
	{
		MyFName ftag(tag);
		if (gameObjects_tag.end() == gameObjects_tag.find(ftag)) {
			// 指定されたタグがタグ持ちゲームオブジェクト配列にない
			return GameObjectList();	 // 見つからなかったので空のGameObjectListを返す
		}

		return gameObjects_tag.at(ftag);
	}


	/**
	* タグ持ちゲームオブジェクト配列に追加する
	*
	* @param tagName	新しく設定するタグ
	* @param gameObj	ゲームオブジェクトポインタ
	* 
	* @return 追加に成功したか
	*/
	inline bool GameObjectAddTag(
		const MyFName& tagName, const GameObject* gameObj)
	{
		// ゲームオブジェクト配列から自身のシェアードポインタを探す
		const auto& itr = std::find_if(gameObjects.begin(), gameObjects.end(),
			[&gameObj](const GameObjectPtr& gObj) {
				return gObj.get() == gameObj; });

		// 見つからなかったら処理しない
		if (itr == gameObjects.end()) {
			return false;
		}

		// タグ持ちゲーム配列に追加
		gameObjects_tag[tagName].push_back(*itr);
		return true;
	}

	/**
	* タグ持ちゲームオブジェクト配列から指定タグのオブジェクトを削除する
	*
	* @param tagName	削除するオブジェクトの持つタグ
	* @param gameObj	ゲームオブジェクト配列のポインタ
	*
	* @return 削除に成功したか
	*/
	inline bool GameObjectRemoveTag(
		const MyFName& tagName, const GameObject* gameObj)
	{
		const auto& objs = gameObjects_tag.find(tagName);
		if (objs == gameObjects_tag.end()) {
			// タグが存在しなければ飛ばす
			return false;
		}

		// タグゲームオブジェクト配列の中にある自身を探す
		const auto& itr = 
			std::find_if(objs->second.begin(), objs->second.end(),
			[&gameObj](const GameObjectPtr& gObj) {
				return gObj.get() == gameObj; });

		// 見つからなかったら処理しない
		if (itr == gameObjects.end()) {
			return false;
		}

		// タグゲームオブジェクト配列から削除
		objs->second.erase(itr);
		return true;
	}

	// すべてのゲームオブジェクトを削除する
	inline void ClearGameObjectAll() {
		for (const auto& e : gameObjects) {
			e->OnDestroy();
		}
		gameObjects.clear();
		gameObjects_tag.clear();

		mainCamera.reset();
		directionalLight.reset();
	}

public:	// 全ゲームオブジェクトとの当たり判定
	/**
	* 光線とすべてのゲームオブジェクトのコライダーとの交差判定
	*
	* @param ray            光線
	* @param hitInfo        光線と最初に交差したコライダーの情報
	* @param pred           交差判定を行うコライダーを選別する述語
	*
	* @return いずれかのコライダーと交差したか
	*/
	inline bool Raycast_GameObjectAll(
		const Ray& ray, RayHitInfo& hitInfo, const RaycastPredicate& pred) 
	{
		return Raycast(ray, gameObjects, hitInfo, pred);
	}

	/**
	*  点とすべてのゲームオブジェクトのコライダーとの接触判定
	*
	* @param point          点の座標
	* @param pred           交差判定を行うコライダーを選別する述語
	*
	* @return 点と接触しているコライダーポインタ配列
	*/
	inline ColliderList InsidePoint_GameObjectAll(
		const vec3 point, const InsidePointPredicate& pred) 
	{
		return InsidePoint(point, gameObjects, pred);
	}

public:	// シーン
	// 現在のシーンを取得する
	inline Scene* GetCurrentScene() { return scene.get(); }

	// 次のシーンを設定する
	template<typename S>
	inline void SetNextScene() { nextScene = std::make_shared<S>(); }

public:	// ウィンドウ
	// ウィンドウがフォーカスされているか
	inline bool FocusWindow() const {
		return glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
	}

public:	// マウスカーソル
	// マウスカーソルの表示状態を取得
	inline bool const GetShowMouseCursor() { return showMouseCursor; }

	// マウスカーソルの非表示を設定
	inline void SetShowMouseCursor(const bool& isVisible) {
		showMouseCursor = isVisible;

		// デバッグウィンドウが表示されていなかったら即時適応する
		if (!debugWindow) {
			glfwSetInputMode(window, GLFW_CURSOR,
				isVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		}
	}

public:	// メインカメラ
	// メインカメラを取得する
	inline GameObject* GetMainCameraObject() {
		if (mainCamera) {
			// メインカメラが登録されている
			return mainCamera.get();
		}
		return nullptr;
	}
	inline const GameObject* GetMainCameraObject() const {
		if (mainCamera) {
			// メインカメラが登録されている
			return mainCamera.get();
		}
		return nullptr;
	}

	// メインカメラを設定する
	inline void SetMainCameraObject(const GameObjectPtr& cameraObject) {
		// カメラコンポーネントのある配列要素番号を取得
		size_t cameraCompIndex = 0;
		for (const auto& e : cameraObject->components) {
			if (std::dynamic_pointer_cast<Camera>(e)) {
				break;
			}
			++cameraCompIndex;
		}

		if (cameraCompIndex == cameraObject->components.size()) {
			// カメラコンポーネントが見つからなかった
			LOG_WARNING("カメラコンポーネントが設定されていません");
			return;
		}

		// カメラコンポーネントがコンポーネント配列の最後尾ではなかったら最後尾へ移動させる
		if (cameraCompIndex != cameraObject->components.size() - 1) {
			std::swap(
				cameraObject->components[cameraCompIndex], cameraObject->components.back());
		}

		mainCamera = cameraObject;
	}

	// メインカメラのカメラコンポーネントを取得する
	inline Camera* GetMainCamera_CameraComponent() const {
		if (const auto& camera = GetMainCameraObject()) {
			// コンポーネントの一番後ろにあるカメラコンポーネントを取得
			return dynamic_cast<Camera*>(camera->components.back().get());
		}
		return nullptr;
	}

public:	// フレームバッファ
	// フレームバッファの大きさを取得する
	vec2Int GetFlamebufferSize() const;

	// フレームバッファのアスペクト比を取得する
	float GetAspectRatio() const;

public:	// メッシュバッファ
	/**
	* スタティックメッシュの取得
	*
	* @param name メッシュ名
	*
	* @return 名前がnameと一致するスタティックメッシュ
	*/
	StaticMeshPtr GetStaticMesh(const char* name);

public:	//Gltfバッファ
	/**
	* glTFファイルの取得
	*
	* @param name glTfファイルの名前
	*
	* @return 名前がnameと一致するglTfファイル
	*/
	GltfFilePtr GetGltfFile(const char* name);

	/**
	* アニメーションメッシュの描画用データを追加
	*
	* @param matBones SSBOに追加する姿勢行列の配列
	+*
	* @return matBones用に割り当てられたSSBOの範囲
	*/
	GltfFileRange AddAnimationMatrices(const AnimationMatrices& matBones);

	/**
	* アニメーションメッシュの描画に使うSSBO領域を割り当てる
	*
	* @param bindingPoint バインディングポイント
	* @param range        バインドする範囲
	*/
	void BindAnimationBuffer(GLuint bindingPoint, const GltfFileRange& range);
	
public:	// ライトバッファ
	/**
	* 新しいライトを取得する
	*
	* @param lightPtr ライトコンポーネントポインタ
	*/
	void AllocateLight(const LightPtr& lightPtr);

public:	// パーティクルバッファ
	/**
	* エミッターを追加する
	*
	* @param  emitterParam  エミッターの初期化パラメータ
	*
	* @return 追加したエミッター
	*/
	ParticleEmitterPtr AddParticleEmitter(const ParticleEmitterParameter& emitterParam);

	/**
	* 指定された名前を持つエミッターを検索する
	*
	* @param name  検索するID
	*
	* @return 引数idと一致するIDを持つエミッター
	*/
	ParticleEmitterPtr FindParticleEmitter(const char* name) const;

	/**
	* 指定されたエミッターを削除する
	*/
	void RemoveParticleEmitter(const ParticleEmitterPtr& emitterPtr);

	/**
	* すべてのエミッターを削除する
	*/
	void RemoveParticleEmitterAll();

private:
	// フレームバッファと同じサイズのFBOを作成する
	void MatchFboSize();
	// カラーテクスチャをFBOに描画する
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