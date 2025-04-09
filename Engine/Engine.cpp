/**
* @file Engine.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Engine.h"

#undef APIENTRY
#include <Windows.h>
#undef far
#undef near
#undef max
#undef min
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../Utility/MyGameFunction.h"
#include "../UniformLocation.h"
#include "DebugLog.h"
#include "EngineSupport/LoadIcon.h"
#include "EngineSupport/ColliderDetection.h"
#include "EngineSupport/FreeRoamCamera.h"

#include "GraphicsObject/ProgramPipeline.h"
#include "GraphicsObject/FramebufferObject.h"
#include "GraphicsObject/VertexArrayObject.h"
#include "Log/LogBuffer.h"
#include "EasyAudio/EasyAudio.h"
#include "Mesh/MeshBuffer.h"
#include "Gltf/GltfFileBuffer.h"
#include "Light/LightBuffer.h"
#include "Particle/ParticleBuffer.h"
#include "Texture/TextureManager.h"
#include "Input/InputManager.h"
#include "Input/InputSystem.h"
#include "Input/InputActionMap.h"

#include "../Component/Camera.h"
#include "../Component/Rigidbody.h"
#include "../Component/Billboard.h"
#include "../Component/DirectionalLight.h"
#include "../Component/FallStone.h"
#include "../Component/SpriteRenderer.h"
#include "../Component/MeshRenderer.h"
#include "../Component/UILayout.h"
#include "../Component/BoxCollider.h"

#include "../Scene/DebugSnene.h"
#include "../Scene/TitleScene.h"
#include "../Scene/LoadMainGameScene.h"
#include "../Scene/MainGameScene.h"
#include "../Scene/BackToTitleScene.h"

/**
* OpenGLからのメッセージを処理するコールバック関数
*
* @param source    メッセージの発信者(OpenGL、Windows、シェーダなど)
* @param type      メッセージの種類(エラー、警告など)
* @param id        メッセージを一位に識別する値
* @param severity  メッセージの重要度(高、中、低、最低)
* @param length    メッセージの文字数. 負数ならメッセージは0終端されている
* @param message   メッセージ本体
* @param userParam コールバック設定時に指定したポインタ
*/
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (type != GL_DEBUG_TYPE_ERROR) {	//エラー以外なら行わない
		return;
	}

	std::string s;
	if (length < 0) {
		//メッセージは終端(終わりが保障されている)のでそのまま代入する
		s = message;
	}
	else {
		//始まり(message)から終わり(message + length)分代入する
		s.assign(message, message + length);
	}
	s += '\n'; // メッセージには改行がないので追加する
	//文字列を出力する
	LOG(s.c_str());
}


/**
* ゲームエンジンを実行する
*
* @retval 0     正常に実行が完了した
* @retval 0以外 エラーが発生した
*/
int Engine::Run()
{
	{
		const int result = Initialize();
		if (result) {
			return result;
		}
	}

	// 音声ライブラリを初期化
	if (!EasyAudio::Initialize()) {
		return 1; // 初期化失敗
	}
	
	float changeScreenModeInterval = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		// 強制終了
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			if (!debugWindow && !showMouseCursor) {
				// カーソルが表示する
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}


			int ms = MessageBox(NULL, "Do you want to exit the game?",
				"Execution Confirmation", MB_ICONEXCLAMATION | MB_OKCANCEL);
			if (ms == IDOK) {
				break;
			}
			else {
				if (!debugWindow && !showMouseCursor) {
					// カーソルを非表示にする
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
			}
		}

		// スクリーンモードの切り替え
		if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && 
			changeScreenModeInterval <= 0) 
		{
			if (isFullScreen) {
				// ウィンドウモードへ変更
				glfwSetWindowMonitor(window, nullptr, 100, 100, 1280, 720, 60);
				isFullScreen = false;
			}
			else {
				// フルスクリーンへ変更
				GLFWmonitor* monitor = glfwGetPrimaryMonitor();	// モニターを取得
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);	// ビデオモードを取得
				glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

				MatchFboSize();	// FBOを作成しなおす
				isFullScreen = true;
			}

			changeScreenModeInterval = 1.0f;
		}

		// デバックウィンドウ表示・非表示の切り替え
		if (!debugWindow) {
			// デバックウィンドウが閉じていたら
			if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
				// カーソルを表示にする
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

				// デバックウィンドウを作成する
				glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
				debugWindow = 
					glfwCreateWindow(1000, 750, debugTitle.c_str(), nullptr, nullptr);

				if (debugWindow) {
					// アイコン画像を読み込んでアイコンに設定
					const GLFWimage& iconImage =
						LoadIcon_as_GLFWimage("Res/Icon/DebugWindow.ico");
					if (iconImage.pixels) {
						glfwSetWindowIcon(debugWindow, 1, &iconImage);
					}

					// ImGuiの初期化
					ImGui::CreateContext();
					ImGui_ImplGlfw_InitForOpenGL(debugWindow, true);
					const char* glsl_version = "#version 450";
					ImGui_ImplOpenGL3_Init(glsl_version);
				}
			}
		}
		else {
			// デバックウィンドウが開いていたら

			// 強制終了
			if (glfwGetKey(debugWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				break;
			}

			// デバックウィンドウを閉じる
			if (glfwWindowShouldClose(debugWindow)) {
				// カーソルがを元に戻す
				if (!showMouseCursor) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}

				// デバックウィンドウを終了する
				glfwDestroyWindow(debugWindow);
				debugWindow = nullptr;

				// ImGuiの終了
				ImGui_ImplOpenGL3_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();
			}
		}

		glfwMakeContextCurrent(window);

		// FPSを計測
		const double curTime_s = glfwGetTime();

		const double diffTime_s = curTime_s - fpsTime;
		if (diffTime_s >= 1) {
			// 1秒ごとにFPSを計算
			fps = static_cast<float>(fpsCount / diffTime_s);

			// 計測用変数をリセット
			fpsTime = curTime_s;
			fpsCount = 0;
		}

		// デルタタイム(前回の更新からの経過時間)を計算
		deltaSeconds = static_cast<float>(curTime_s - previousTime_s);

		// 経過時間が長すぎる場合は適当に短くする(主にデバッグ対策)
		if (deltaSeconds >= 0.5f) {
			deltaSeconds = oneFpsTime_s;
		}

		if (deltaSeconds >= oneFpsTime_s) {	// 更新間隔
			previousTime_s = curTime_s;
			++fpsCount;

			Update();

			if (mainCamera) {
				Render();
			}
		}

		// 音声ライブラリを更新
		EasyAudio::Update();

		if (debugWindow) {
			RenderImGui();
		}

		// フルスクリーンインターバル
		if (changeScreenModeInterval > 0) {
			changeScreenModeInterval -= deltaSeconds;
		}

		//OSからの要求を処理(マウスやキーボードなどの状態を取得)
		glfwPollEvents();
	}

	// フリーカメラを削除
	if (freeCamera) {
		delete freeCamera;
	}

	// 音声ライブラリを終了
	EasyAudio::Finalize();

	// GLFWの終了
	glfwTerminate();	//ウィンドウを終了する際必ず呼び出す

	return 0;
}

/**
* コライダーの状況に応じた色を取得する
*/
vec4 Engine::GetCollisionColor(const Collider& collider)
{
	if (!collider.isActive) return collisionColor[1];
	if (collider.isTrigger) return collisionColor[2];
	if (collider.isStatic) return collisionColor[3];
	return collisionColor[0];
}

/**
* UIオブジェクトを作成する
*
* @param name		UIオブジェクトの名前
* @param filename	UIオブジェクトに表示する画像
* @param magnification    UIオブジェクトの大きさ
*
* @return 作成したUIオブジェクト
*/
Engine::UIObject Engine::CreateUIObject(
	const std::string& name, const char* filename,
	const float& magnification)
{
	UIObject uiObj;

	uiObj.object = Create<GameObject>(name);
	uiObj.object->renderQueue = RenderQueue_overlay;
	uiObj.object->layer = Layer_UI;

	uiObj.spriteRender = uiObj.object->AddComponent<SpriteRenderer>();
	uiObj.spriteRender->material.texBaseColor = 
		TextureManager::GetTexture(filename, Usage::for2D);

	// 画像サイズに応じて拡大率を調整
	const float aspectRatio = 
		uiObj.spriteRender->material.texBaseColor->GetAspectRatio();
	uiObj.spriteRender->scale = { magnification * aspectRatio, magnification, 1 };

	// コンポーネントを追加
	uiObj.uiLayot = uiObj.object->AddComponent<UILayout>();

	return uiObj;
}

/**
* フレームバッファの大きさを取得する
*
* @return フレームバッファの縦と横のサイズ
*/
vec2Int Engine::GetFlamebufferSize() const
{
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	return { w, h };
}

/**
* フレームバッファのアスペクト比を取得する
*
* @return フレームバッファのアスペクト比
*/
float Engine::GetAspectRatio() const
{
	const vec2 size = vec2(GetFlamebufferSize());
	return size.x / size.y;
}

/**
* スタティックメッシュの取得
*
* @param name メッシュ名
*
* @return 名前がnameと一致するスタティックメッシュ
*/
StaticMeshPtr Engine::GetStaticMesh(const char* name)
{
	return meshBuffer->GetStaticMesh(name);
}


/**
* glTFファイルの取得
*
* @param name glTfファイルの名前
*
* @return 名前がnameと一致するglTfファイル
*/
GltfFilePtr Engine::GetGltfFile(const char* name)
{
	return gltfFileBuffer->GetGltfFile(name);
}

/**
* アニメーションメッシュの描画用データを追加
*
* @param matBones SSBOに追加する姿勢行列の配列
+*
* @return matBones用に割り当てられたSSBOの範囲
*/
GltfFileRange Engine::AddAnimationMatrices(const AnimationMatrices& matBones)
{
	return gltfFileBuffer->AddAnimationMatrices(matBones);
}

/**
* アニメーションメッシュの描画に使うSSBO領域を割り当てる
*
* @param bindingPoint バインディングポイント
* @param range        バインドする範囲
*/
void Engine::BindAnimationBuffer(GLuint bindingPoint, const  GltfFileRange& range)
{
	gltfFileBuffer->BindAnimationBuffer(bindingPoint, range);
}


/**
* 新しいライトを取得する
*
* @param lightPtr ライトコンポーネントポインタ
*/
void Engine::AllocateLight(const LightPtr& lightPtr)
{
	lightBuffer->Allocate(lightPtr);
}


/**
* エミッターを追加する
*
* @param  emitterParam  エミッターの初期化パラメータ
*
* @return 追加したエミッター
*/
ParticleEmitterPtr Engine::AddParticleEmitter(const ParticleEmitterParameter& emitterParam)
{
	return particleBuffer->AddEmitter(emitterParam);
}

/**
* 指定された名前を持つエミッターを検索する
*
* @param name  検索するID
*
* @return 引数idと一致するIDを持つエミッター
*/
ParticleEmitterPtr Engine::FindParticleEmitter(const char* name) const
{
	return particleBuffer->FindEmitter(name);
}

/**
* 指定されたエミッターを削除する
*/
void Engine::RemoveParticleEmitter(const ParticleEmitterPtr& emitterPtr)
{
	particleBuffer->RemoveEmitter(emitterPtr);
}

/**
* すべてのエミッターを削除する
*/
void Engine::RemoveParticleEmitterAll()
{
	particleBuffer->RemoveEmitterAll();
}


/**
* FBOのサイズをフレームバッファのサイズに合わせる
*/
void Engine::MatchFboSize()
{
	// フルスクリーンなら変化することはないため処理しない
	if (isFullScreen) {
		return;
	}

	// ビューポートをフレームバッファのサイズに合わせる
	const auto& texGameWindow = fboMainGameWindow->GetColorTexture();
	const vec2Int fboFrame = {
		texGameWindow->GetWidth(),
		texGameWindow->GetHeight()
	};

	const vec2Int currentFlame = GetFlamebufferSize();

	// フレームバッファとFBOの大きさが異なる場合はFBOを作り直す
	if (currentFlame != fboFrame) {

		// シェアードポインタを上書きすることでFBOを破棄する

		// ゲームウィンドウ用FBOを作成
		const auto& texGameWindowColor = TextureManager::CreateTexture(
			"FBO(color)", currentFlame.x, currentFlame.y, GL_RGBA16F);
		const auto& texGameWindowDepth = TextureManager::CreateTexture(
			"FBO(depth)", currentFlame.x, currentFlame.y, GL_DEPTH_COMPONENT32F);
		fboMainGameWindow = FramebufferObject::Create(texGameWindowColor, texGameWindowDepth);

		// エミッション用FBOを作成
		const auto& texEmissionColor = TextureManager::CreateTexture(
			"FBO(color)", currentFlame.x, currentFlame.y, GL_RGBA16F);
		/* ゲームウィンドウと同じ深度テクスチャを使用 */
		fboEmission = FramebufferObject::Create(texEmissionColor, texGameWindowDepth);

		// ブルーム用FBOを作成
		vec2Int bloom = currentFlame;
		for (auto& bfbo : fboBloom) {
			bloom /= 2;
			const auto& texBloom = TextureManager::CreateTexture(
				"FBO(color)", bloom.x, bloom.y, GL_RGBA16F);
			bfbo = FramebufferObject::Create(texBloom, nullptr);
		}

		// SAO用FBOを作成
		// 深度値の縮小バッファ
		const int maxMipLevel = static_cast<int>(fboSAODepth.size());
		const auto& texSAODepth = TextureManager::CreateTexture(
			"FBO(sao depth)", currentFlame.x / 2, currentFlame.y / 2, GL_R32F, maxMipLevel);
		for (int level = 0; level < maxMipLevel; ++level) {
			fboSAODepth[level] =
				FramebufferObject::Create(texSAODepth, nullptr, level, 0);
		}
		// 計算結果バッファ
		const auto& texSAO = TextureManager::CreateTexture(
			"FBO(sao)", currentFlame.x / 2, currentFlame.y / 2, GL_R8);
		fboSAOCalc = FramebufferObject::Create(texSAO, nullptr);
		// ぼかしバッファ
		const auto& texSAOBlur = TextureManager::CreateTexture(
			"FBO(sao blur)", currentFlame.x / 2, currentFlame.y / 2, GL_R8);
		fboSAOBlur = FramebufferObject::Create(texSAOBlur, nullptr);

	}
}

/**
* カラーテクスチャをFBOに描画する
*
* @param fbo      描画先FBO
* @param texture  FBOに描画するテクスチャ
* @param wrapMode テクスチャに設定するラップモード
*/
void Engine::DrawTextureToFbo(
	FramebufferObject& fbo, ProgramPipeline& prog, 
	TexturePtr texture, GLenum wrapMode)
{
	glUseProgram(prog);

	// VAOとシェーダをOpenGLコンテキストに割り当てる
	glBindVertexArray(*meshBuffer->GetVAO());

	// 描画先FBOを割り当てる
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	const Texture& texFbo = *fbo.GetColorTexture();
	glViewport(0, 0, texFbo.GetWidth(), texFbo.GetHeight());

	// ラップモードを変更
	texture->SetWrapMode(wrapMode);

	// カラーテクスチャを使用したマテリアルを作成
	Material mt;
	mt.texBaseColor = texture;

	// 描画先FBOにテクスチャを描画
	DrawSpritePlaneMesh(*windowMesh, prog, vec4(1), mt);

	glBindVertexArray(0);
}

/**
* ゲームエンジンを初期化する
*
* @retval 0     正常に初期化された
* @retval 0以外 エラーが発生した
*/
int Engine::Initialize()
{
	// GLFWの初期化
	if (glfwInit() != GLFW_TRUE) {
		return 1; // 初期化失敗
	}

	// 描画ウィンドウの作成
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	window = glfwCreateWindow(1280, 720, title.c_str(), nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return 1; // ウィンドウ作成失敗
	}

	// アイコン画像を読み込んでアイコンに設定
	const GLFWimage& iconImage = LoadIcon_as_GLFWimage("Res/Icon/Dragon.ico");
	if (iconImage.pixels) {
		glfwSetWindowIcon(window, 1, &iconImage);
	}

	// OpenGLコンテキストの作成
	glfwMakeContextCurrent(window);

	// OpenGL関数のアドレスを取得
	//GLFWとGLADでは関数型が異なるため「reinterpret_cast」でGLADライブラリの型にキャスト
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		glfwTerminate();
		return 1; // アドレス取得失敗
	}

	// メッセージコールバックの設定
	glDebugMessageCallback(DebugCallback, nullptr);

	// 2つのシェーダをリンク
	{
		// ゲームウィンドウシェーダ
		progWindow = 
			ProgramPipeline::Create("Simple2D.vert", "Simple2D.frag");

		// 通常シェーダ
		progDefaultLit =
			ProgramPipeline::Create("DefaultLit.vert", "DefaultLit.frag");
		// アンリット
		progUnlit =		
			ProgramPipeline::Create("Unlit.vert", "Unlit.frag");
		// アニメーション
		progAnimationLit =	
			ProgramPipeline::Create("AnimationLit.vert", "DefaultLit.frag");
		// アニメーションアンリット
		progAnimationUnlit =
			ProgramPipeline::Create("AnimationLit.vert", "Unlit.frag");
		// スペキュラ
		progSprcular = 
			ProgramPipeline::Create("DefaultLit.vert", "Sprcular.frag");
		// 落下石の影を反映させるた通常シェーダ
		progDefaultLit_with_StoneShadow =
			ProgramPipeline::Create("DefaultLit.vert", "DefaultLit_with_StoneShadow.frag");	

		// 光の影響を受けるシェーダを配列に入れる
		affectedLightPrograms = { 
			*progDefaultLit, *progAnimationLit, *progSprcular, *progDefaultLit_with_StoneShadow,
		};

		// 影用シェーダ
		progShadow =	// シャドウ
			ProgramPipeline::Create("Shadow/Shadow.vert", "Shadow/Shadow.frag");	
		progShadowAnimation =	// アニメーションシャドウ
			ProgramPipeline::Create("Shadow/ShadowAnimation.vert", "Shadow/Shadow.frag");	
		glProgramUniform1f(*progShadow, Loc::AlphaCutoff, 0.5f); // カットオフ値を設定しておく

		// エミッション用シェーダ
		progEmission = 
			ProgramPipeline::Create("Unlit.vert", "Emission.frag");
		progEmissionAnimation = 
			ProgramPipeline::Create("AnimationUnlit.vert", "Emission.frag");

		// ブルーム用シェーダ
		progHighPassFilter = 
			ProgramPipeline::Create("Simple2D.vert", "Bloom/HighPassFilter.frag");
		progDownSampling = 
			ProgramPipeline::Create("Simple2D.vert", "Bloom/DownSampling.frag");
		progUpSampling = 
			ProgramPipeline::Create("Simple2D.vert", "Bloom/UpSampling.frag");

		// アンビエントオクルージョン用シェーダ
		progSAORestoreDepth = 
			ProgramPipeline::Create("Simple2D.vert", "AmbientOcclusion/SaoRestoreDepth.frag");
		progSAODownSampling = 
			ProgramPipeline::Create("Simple2D.vert", "AmbientOcclusion/SaoDownSampling.frag");
		progSAOCalc = 
			ProgramPipeline::Create("Simple2D.vert", "AmbientOcclusion/SaoCalc.frag");
		progSAOBlur = 
			ProgramPipeline::Create("Simple2D.vert", "AmbientOcclusion/SaoBlur.frag");
		progSAO = 
			ProgramPipeline::Create("Simple2D.vert", "AmbientOcclusion/Sao.frag");

		// ゲームウィンドウやポストエフェクトシェーダに
		// 座標変換行列をユニフォーム変数にコピー
		// (スプライトとFBOのテクスチャ座標は逆なのでY軸をマイナスにする)
		const mat4 matModel = {
			vec4(1, 0, 0, 0),
			vec4(0,-1, 0, 0),
			vec4(0, 0, 1, 0),
			vec4(0, 0, 0, 1),
		};
		const mat4 matVewProjection = mat4(1);
		const GLuint programIdList[] = {
			 *progWindow,
			 *progHighPassFilter, *progDownSampling, *progUpSampling,
			 *progSAORestoreDepth, * progSAODownSampling, *progSAOCalc, *progSAOBlur, *progSAO
		};
		for (const auto& prog : programIdList) {
			glProgramUniformMatrix4fv(prog, 0, 1, GL_FALSE, &matModel[0][0]);
			glProgramUniformMatrix4fv(prog, 1, 1, GL_FALSE, &matVewProjection[0][0]);
		}
	}

	// FBOを作成
	{
		// ゲームウィンドウ用FBOを作成
		const vec2Int flame = GetFlamebufferSize();
		const auto& texGameWindowColor = TextureManager::CreateTexture(
			"FBO(color)", flame.x, flame.y, GL_RGBA16F);
		const auto& texGameWindowDepth = TextureManager::CreateTexture(
			"FBO(depth)", flame.x, flame.y, GL_DEPTH_COMPONENT32F);
		fboMainGameWindow = FramebufferObject::Create(texGameWindowColor, texGameWindowDepth);

		// 影用FBOを作成
		const auto& texShadow = TextureManager::CreateTexture(
			"FBO(shadow)", 2048, 2048, GL_DEPTH_COMPONENT32);
		fboShadow = FramebufferObject::Create(nullptr, texShadow);

		// エミッション用FBOを作成
		const auto& texEmissionColor = TextureManager::CreateTexture(
			"FBO(emission)", flame.x, flame.y, GL_RGBA16F);
		/* ゲームウィンドウと同じ深度テクスチャを使用 */
		fboEmission = FramebufferObject::Create(texEmissionColor, texGameWindowDepth);

		// ブルーム用FBOを作成
		vec2Int bloom = flame;
		for (auto& bfbo : fboBloom) {
			bloom /= 2;
			const auto& texBloom = TextureManager::CreateTexture(
				"FBO(bloom)", bloom.x, bloom.y, GL_RGBA16F);
			bfbo = FramebufferObject::Create(texBloom, nullptr);
		}

		// SAO用FBOを作成する
		// 深度値の縮小バッファ
		const int maxMipLevel = static_cast<int>(fboSAODepth.size());
		const auto& texSAODepth = TextureManager::CreateTexture(
			"FBO(sao depth)", flame.x / 2, flame.y / 2, GL_R32F, maxMipLevel);
		for (int level = 0; level < maxMipLevel; ++level) {
			fboSAODepth[level] = 
				FramebufferObject::Create(texSAODepth, nullptr, level, 0);
		}
		// 計算結果バッファ
		const auto& texSAO = TextureManager::CreateTexture(
			"FBO(sao)", flame.x / 2, flame.y / 2, GL_R8);
		fboSAOCalc = FramebufferObject::Create(texSAO, nullptr);
		// ぼかしバッファ
		const auto& texSAOBlur = TextureManager::CreateTexture(
			"FBO(sao blur)", flame.x / 2, flame.y / 2, GL_R8);
		fboSAOBlur = FramebufferObject::Create(texSAOBlur, nullptr);
	}

	// バッファを作成
	meshBuffer = MeshBuffer::Create(10'000'000);
	lightBuffer = LightBuffer::Create();
	particleBuffer = ParticleBuffer::Create(1'000);

	// glTFファイル用のバッファを作成
	const size_t maxAnimeModelCount = 8;   // アニメーションするモデル数
	const size_t maxAnimeMatrixCount = 128; // 1モデルのボーン数
	gltfFileBuffer = 
		GltfFileBuffer::Create(10'000'000, maxAnimeModelCount * maxAnimeMatrixCount);


	windowMesh = meshBuffer->CreatePlaneXY("PlaneXY");

	// OBJファイルを読み込む
	skySphere = 
		meshBuffer->LoadOBJ("Res/Obj/SkySphere/SkySphere.obj");

	meshBuffer->LoadOBJ("Res/Obj/Default/Box.obj");
	meshBuffer->LoadOBJ("Res/Obj/Default/Plane.obj");
	meshBuffer->LoadOBJ("Res/Obj/Default/Sphere.obj");

	for (int i = 0; i <= 5; ++i) {
		meshBuffer->LoadOBJ(
			std::string("Res/Obj/Stone/Stone" + std::to_string(i) + ".obj").c_str());
	}

	meshBuffer->LoadOBJ("Res/Obj/Sword/Sword.obj");
	meshBuffer->LoadOBJ("Res/Obj/Stage/Stage.obj");
	meshBuffer->LoadOBJ("Res/Obj/Turntable/Turntable.obj");
	meshBuffer->LoadOBJ("Res/Obj/SpotLight/SpotLight.obj");
	meshBuffer->LoadOBJ("Res/Obj/Propeller/Propeller.obj");


	// glTFファイルを読み込む
	gltfFileBuffer->LoadGltf("Res/Gltf/Dragon/Dragon.gltf");
	gltfFileBuffer->LoadGltf("Res/Gltf/Hero/Hero.gltf");


	// インプットの初期処理
	InputManager::GetInstance().StartUp(window);
	// インプットシステムへアクション登録
	{
		// UIアクションマップを作成
		{
			InputActionMap uiMap("UI");
			{
				// タイミングリング
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_LEFT },
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_RIGHT	},
					{ InputKind::PadTrigger,	GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER	},
				};
				uiMap.RegisterAction("TimingRing", action);
			}

			// UIアクションマップを追加
			InputSystem::GetInstance().actionMaps.push_back(uiMap);
		}

		// プレイヤーアクションマップを作成
		{
			InputActionMap playerMap("Player");
			{
				// ジャンプ
				std::vector<InputActionData> action = {
					{ InputKind::Key,		GLFW_KEY_SPACE },
					{ InputKind::PadButton, GLFW_GAMEPAD_BUTTON_A },
					{ InputKind::PadButton, GLFW_GAMEPAD_BUTTON_B },
				};
				playerMap.RegisterAction("Jump", action);
			} {
				// 走る
				std::vector<InputActionData> action = {
					{ InputKind::Key,		GLFW_KEY_LEFT_SHIFT },
					{ InputKind::Key,		GLFW_KEY_RIGHT_SHIFT },
					{ InputKind::PadButton, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER },
				};
				playerMap.RegisterAction("Run", action);
			} {
				// 弱攻撃
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_LEFT },
					{ InputKind::PadButton,		GLFW_GAMEPAD_BUTTON_X },
				};
				playerMap.RegisterAction("Attack", action);
			} {
				// 強攻撃
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_RIGHT	},
					{ InputKind::PadButton,		GLFW_GAMEPAD_BUTTON_Y },
				};
				playerMap.RegisterAction("StrongAttack", action);
			} {
				// タイミングリング
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_LEFT },
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_RIGHT	},
					{ InputKind::PadTrigger,	GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER	},
				};
				playerMap.RegisterAction("TimingRing", action);
			} {
				// ターゲットフォーカス
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_MIDDLE },
					{ InputKind::PadButton,		GLFW_GAMEPAD_BUTTON_RIGHT_THUMB	},
				};
				playerMap.RegisterAction("Targeting", action);
			} 
			
			{
				// 移動
				std::vector<InputActionData> action = {
					{ InputKind::MoveAction },
				};
				playerMap.RegisterAction("Move", action);
			} {
				// 視点
				std::vector<InputActionData> action = {
					{ InputKind::ViewAction },
				};
				playerMap.RegisterAction("View", action);
			}

			// プレイヤーアクションマップを追加
			InputSystem::GetInstance().actionMaps.push_back(playerMap);
		}
	}

	// ゲームオブジェクト配列の容量を予約
	gameObjects.reserve(500);
	gameObjects_tag.reserve(100);

	return 0; // 正常に初期化された
}

/**
* ゲームエンジンの状態を更新する
*/
void Engine::Update()
{
	// インプットの状態を更新
	InputManager::GetInstance().UpdeteStates(deltaSeconds);

	// フリーカメラの更新
	if (freeCamera) {
		freeCamera->Update(deltaSeconds, *this);
	}

	// ゲーム内の前回フレームからの経過時間
	const float gameDeltaSeconds = 
		deltaSeconds * gameSpeed * timeScale;

	// シーンの切り替え
	if (nextScene) {
		if (scene) {
			scene->Finalize(*this);
		}

		nextScene->Initialize(*this);
		
		scene = std::move(nextScene);
	}

	// シーンを更新
	if (scene) {
		scene->Update(*this, gameDeltaSeconds);
	}

	// すべてのゲームオブジェクトの更新処理
	UpdateGameObject(gameDeltaSeconds);

	// 削除指定されているゲームオブジェクトを削除
	RemoveDestroyedGameObject();

	// メインカメラの生存確認
	if (mainCamera) {
		if (mainCamera->IsDestroyed()) {
			// 削除されているためリセット
			mainCamera.reset();
		}
	}

	// メインカメラのカメラコンポーネントの生存確認
	if (mainCamera) {
		if (!GetMainCamera_CameraComponent()) {
			// カメラコンポーネントが取得できなかったら
			// 現在のメインカメラにカメラコンポーネントがあるか探す
			size_t cameraCompIndex = 0;
			for (const auto& e : mainCamera->components) {
				if (std::dynamic_pointer_cast<Camera>(e)) {
					break;
				}
				++cameraCompIndex;
			}

			// カメラコンポーネントがなかったらメインカメラをリセットする
			if (cameraCompIndex == mainCamera->components.size()) {
				mainCamera.reset();
			}
			else {
				// カメラコンポーネントが最後尾へ移動させる
				/* この処理が走っている時点でカメラコンポーネントが最後尾ではない */
				std::swap(
					mainCamera->components[cameraCompIndex], mainCamera->components.back());
			}
		}
	}

	// 平行光源の生存確認
	if (directionalLight) {
		if (directionalLight->IsDestroyed()) {
			// 削除されているためリセット
			directionalLight.reset();
		}
	}


	if (!freeCamera) {
		if (const auto& cameraObj = GetMainCameraObject()) {
			// ライトをGPUメモリにコピーする
			lightBuffer->UpdateShaderLight(cameraObj->GetWorldTransformMatrix());

			Camera* camera = GetMainCamera_CameraComponent();

			// パーティクルを更新
			particleBuffer->Update(camera->GetViewMatrix(), gameDeltaSeconds);

			// 3Dオーディオ用リスナー設定
			const vec3 pos = vec3(cameraObj->GetWorldTransformMatrix()[3]);
			const vec3 rignt = normalize(vec3(cameraObj->GetWorldTransformMatrix()[0]));
			EasyAudio::SetListenr(pos, rignt);
		}
	}
	else {
		// ライトをGPUメモリにコピーする
		lightBuffer->UpdateShaderLight(freeCamera->transMat);

		// パーティクルを更新
		particleBuffer->Update(freeCamera->GetViewMatrix(), gameDeltaSeconds);

		// 3Dオーディオ用リスナー設定
		EasyAudio::SetListenr(freeCamera->position, vec3(freeCamera->transMat[0]));
	}
}

/**
* ゲームオブジェクトの状態を更新する
*
* @param deltaSeconds 前回の更新からの経過時間(秒)
*/
void Engine::UpdateGameObject(float deltaSeconds)
{
	// メインカメラをゲームオブジェクト配列のの先頭に移動する
	/* メインカメラを最初に行うのはメインカメラの情報を基準に処理を行っている場合があるから */
	if (const auto& cameraObj = GetMainCameraObject()) {
		if (gameObjects[0].get() != cameraObj) {
			// 配列の先頭がメインカメラではないため探す
			const auto& mainCamera_itr =
				std::find_if(gameObjects.begin(), gameObjects.end(),
					[&cameraObj](const GameObjectPtr& p) { return p.get() == cameraObj; });

			// 先頭のゲームオブジェクトと入れ替える
			std::swap(*mainCamera_itr, gameObjects[0]);
		}
	}

	// Update関数を実行
	for (int i = 0; i < gameObjects.size(); ++i) {
		const auto& gObj = gameObjects[i];
		if (!gObj->CheckActive()) {
			continue;
		}
		if (gObj->IsDestroyed()) {
			continue;
		}

		gObj->Start();
		gObj->Update(deltaSeconds);
	}

	size_t calcStartIndex = 0;

	// すべてのゲームオブジェクトの座標変換行列を計算
	CalcGameObjectTransMat(calcStartIndex);

	// 現在のすべてのゲームオブジェクトの計算が完了したため配列サイズを保存する
	calcStartIndex = gameObjects.size();

	// EndUpdate関数の実行
	for (int i = 0; i < gameObjects.size(); ++i) {
		const auto& gObj = gameObjects[i];
		if (!gObj->CheckActive()) {
			continue;
		}
		if (gObj->IsDestroyed()) {
			continue;
		}

		gObj->EndUpdate(deltaSeconds);
	}

	// EndUpdateで追加されたゲームオブジェクトの座標変換行列を計算
	CalcGameObjectTransMat(calcStartIndex);
	calcStartIndex = gameObjects.size();


	// ゲームオブジェクトの衝突を処理
	std::vector<WorldColliderList> colliders;	// コライダーのコピー配列
	colliders.reserve(gameObjects.size());

	// ワールド座標系の衝突判定を作成
	for (const auto& gObj : gameObjects) {
		if (!gObj->CheckActive()) {
			continue;
		}
		if (gObj->colliders.empty()) {
			continue;
		}

		// リジッドボディがあれば「接地していない」状態にする
		if (const auto& rigid = gObj->GetRigidbody()) {
			rigid->isGrounded = false;
		}

		// ゲームオブジェクトが保持しているコライダー分の配列を作成
		WorldColliderList wcList;
		wcList.reserve(gObj->colliders.size());
		for (const auto& col : gObj->colliders) {
			if (!col->isActive) {
				continue;
			}

			WorldCollider wc;
			// オリジナルのコライダーをコピー
			wc.origin = col;
			// コライダーの座標をワールド座標に変換
			wc.world = col->GetTransformedCollider(gObj->GetWorldTransformMatrix());

			wcList.push_back(wc);
		}

		//コピーコライダー配列に格納する
		colliders.push_back(wcList);
	}

	// コライダーの衝突処理
	ColliderDetection(&colliders);

	// コライダーの衝突処理で追加されたゲームオブジェクトの座標変換行列を計算
	CalcGameObjectTransMat(calcStartIndex);
} // UpdateGameObject

/**
* ゲームオブジェクトの座標変換行列計算を実行
* 
* @param calcStartIndex	計算を開始するインデックス番号
*/
void Engine::CalcGameObjectTransMat(const size_t& calcStartIndex)
{
	// ローカル座標変換行列を計算
	for (size_t i = calcStartIndex; i < gameObjects.size(); ++i) {
		GameObject* gObj = gameObjects[i].get();
		gObj->localTransMat =
			TransformMatrix(gObj->position, gObj->rotation, gObj->scale);
	}

	// ワールド座標変換行列を計算
	for (size_t i = calcStartIndex; i < gameObjects.size(); ++i) {
		GameObject* gObj = gameObjects[i].get();
		mat4 m = gObj->GetLocalTransformMatrix();

		// 自身のすべての親オブジェクトの座標変換行列と掛け合わせる
		for (gObj = gObj->parent; gObj; gObj = gObj->parent) {
			m = gObj->GetLocalTransformMatrix() * m;
		}

		gameObjects[i]->worldTransMat = m;
	}
}

/**
* 破棄予定のゲームオブジェクトを削除する
*/
void Engine::RemoveDestroyedGameObject()
{
	if (gameObjects.empty()) {
		return; // ゲームオブジェクトを持っていなければ何もしない
	}

	// 破棄予定の有無でゲームオブジェクトを分ける
	const auto& destrObjBegin_itr =
		std::stable_partition(gameObjects.begin(), gameObjects.end(),
			[](const GameObjectPtr& e) { return !e->IsDestroyed(); });

	if (destrObjBegin_itr == gameObjects.end()) {
		return;	// 削除するゲームオブジェクトがない
	}

	// 破棄予定のオブジェクトを別の配列に移動
	GameObjectList destrObjs(
		std::make_move_iterator(destrObjBegin_itr),
		std::make_move_iterator(gameObjects.end()));

	// 配列から移動済みオブジェクトを削除
	gameObjects.erase(destrObjBegin_itr, gameObjects.end());

	// 破棄予定のオブジェクトのOnDestroyを実行と削除したオブジェクトのTagを取得
	std::vector<MyFName> destrObjTags;
	destrObjTags.reserve(destrObjs.size());
	for (const auto& e : destrObjs) {
		e->OnDestroy();

		// オブジェクトのTagを取得し保存
		for (const auto& tag : e->tags) {
			// すでに同じTagが保存されているか調べる
			const auto& itr =
				std::find(destrObjTags.begin(), destrObjTags.end(), tag);
			if (itr != destrObjTags.end()) {
				continue;	// 同じタグが追加済み
			}

			destrObjTags.push_back(tag);
		}

		// メインカメラが削除されるならメインカメラ変数もリセットする
		if (e == mainCamera) {
			mainCamera.reset();
		}
	}

	// Tag持ちゲームオブジェクト配列からも削除する
	for (const MyFName& tag : destrObjTags) {
		// 同じタグを持っているオブジェクトを取得
		const auto& objs = gameObjects_tag.find(tag);
		if (objs == gameObjects_tag.end()) {
			// タグが存在しなければ飛ばす
			continue;
		}

		// 削除指定されている要素を集める
		const auto& begin_itr =
			std::partition(objs->second.begin(), objs->second.end(),
				[](const GameObjectPtr& p) { return !p->IsDestroyed(); });
		if (begin_itr == objs->second.end()) {
			continue;	// 削除するタグオブジェクトがない
		}

		// タグ持ちオブジェクト配列からも削除する
		objs->second.erase(begin_itr, objs->second.end());
	}

	// ここで削除したオブジェクト(destrObjs)が完全に解放される
}



/**
* ゲームエンジンの状態を描画する
*/
void Engine::Render()
{
	// FBOをフレームサイズに合わせる
	MatchFboSize();

	// ゲームウィンドウ用FBOに描画
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);

	// フレームバッファの大きさを取得
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	// ビューポートを設定（ウィンドウサイズが変わっても描画の比率を維持する）　※描画ウィンドウサイズではなく「描画する範囲」を教えている
	glViewport(0, 0, fbWidth, fbHeight);

	// バックバッファを削除するときの色を指定
	glClearColor(0, 0, 0, 1);
	//バックバッファを削除
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 描画に使うシェーダを指定
	glUseProgram(*progDefaultLit);

	// アニメーションバッファをクリア
	gltfFileBuffer->ClearAnimationBuffer();

	// 描画の前処理を実行
	for (const auto& gObj : gameObjects) {
		if (!gObj->CheckActive()) {
			continue;
		}
		if (gObj->IsDestroyed()) {
			continue;
		}

		for (const auto& r : gObj->renderers) {
			if (!r->isActive) {
				continue;
			}
			r->PreDraw(deltaSeconds * gameSpeed * timeScale);
		}
	}

	// アニメーションバッファをGPUメモリにコピーする
	gltfFileBuffer->UploadAnimationBuffer();

	particleBuffer->UpdateSSBO();


	// シェーダにカメラパラメータを設定
	{
		// カメラパラメータを設定するシェーダ配列
		const GLuint programs[] = {
			*progDefaultLit,*progUnlit,
			*progAnimationLit, *progAnimationUnlit,
			*progSprcular, *progDefaultLit_with_StoneShadow,
			*progEmission, *progEmissionAnimation,
			*particleBuffer->progParticle, *particleBuffer->progParticleEmission
		};

		if (!freeCamera) {
			/* 事前にメインカメラの有無をチェックしているためnullチェックはしない */
			GameObject* cameraObj = GetMainCameraObject();
			Camera* camera = GetMainCamera_CameraComponent();

			for (const auto& prog : programs) {
				glProgramUniformMatrix4fv(prog, Loc::CameraTransformMatrix, 1, GL_FALSE,
					&cameraObj->GetWorldTransformMatrix()[0].x);

				glProgramUniform4f(prog, Loc::CameraInfo,
					camera->fovScale.x, camera->fovScale.y, camera->A, camera->B);
			}
		}
		else {
			for (const auto& prog : programs) {
				glProgramUniformMatrix4fv(prog, Loc::CameraTransformMatrix, 1, GL_FALSE,
					&freeCamera->transMat[0].x);

				glProgramUniform4f(prog, Loc::CameraInfo,
					freeCamera->fovScale.x, freeCamera->fovScale.y, freeCamera->A, freeCamera->B);
			}
		}
	}

	// 深度テストを有効化
	glEnable(GL_DEPTH_TEST);

	// 裏面カリングを有効化（裏側を描画しない）
	glEnable(GL_CULL_FACE);

	// 半透明合成を有効化
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// 光源データをGPUメモリにコピー
	{
		// 平行光源のデータを取得
		vec3 color = vec3(0);
		vec3 direction = vec3(0);

		if (isValid(directionalLight)) {
			color =
				directionalLight->color * directionalLight->intensity;
			direction =
				directionalLight->GetDirection();
		}
		else {
			directionalLight.reset();
		}

		for (const auto& prog : affectedLightPrograms) {
			// 環境光
			glProgramUniform3fv(prog, Loc::AmbientLight, 1, &ambientLight.x);

			// 平行光源のパラメータ
			glProgramUniform3fv(prog, Loc::DirectionalLightColor, 1, &color.x);
			glProgramUniform3fv(prog, Loc::DirectionalLightDirection, 1, &direction.x);
		}
	}

	// 落下石データをGPUメモリにコピー
	{
		const GameObjectList& stones = FindGameObjectsWithTag("FallStone");

		// コピーする石があるか
		if (stones.empty()) {
			// ない
			glProgramUniform1i(*progDefaultLit_with_StoneShadow,
				Loc::FallStone::FallStoneCount, 0);
		}
		else {
			// ある
			std::vector<vec4> stoneDatas;
			stoneDatas.reserve(std::min(stones.size(), MaxShaderFallStoneCount));

			// コピーする落下石パラメータを作成
			for (const auto& s :stones) {
				if (!s->CheckActive() ||		// アクティブではない
					!isValid(s) ||				// 削除されている
					s->components.size() == 0)	// コンポーネントがない
				{
					continue;
				}

				// 落下石コンポーネントを取得
				const auto& fs = s->GetComponent<FallStone>();
				if (!fs) {
					continue;
				}

				// パラメーターを追加
				stoneDatas.emplace_back(
					s->position.x, s->position.z,
					fs->param.maxStoneScale,
					1 - (fs->param.maxStoneScale - s->scale.x));

				// 最大コピー数に達したら終了
				if (stoneDatas.size() >= MaxShaderFallStoneCount) {
					break;
				}
			}
			const int cnt = static_cast<int>(stoneDatas.size());
			glProgramUniform1i(*progDefaultLit_with_StoneShadow, 
				Loc::FallStone::FallStoneCount, cnt);
			glProgramUniform4fv(*progDefaultLit_with_StoneShadow, 
				Loc::FallStone::FallStoneInfo, cnt, &stoneDatas[0].x);
		}
	}

	// ゲームオブジェクトをレンダーキュー順に並べ替える
	std::stable_sort(	/* 配列を「並び替えルール」を満たすよう要素を並び変える */
		gameObjects.begin(), gameObjects.end(),
		[](const GameObjectPtr& a, const GameObjectPtr& b) {
			return a->renderQueue < b->renderQueue; });	/*renderQueueが小さい方が前に来るようにする*/

	// 指定されてキューの先頭を検索ラムダ式
	const auto FindRenderQueueBegin =
		[this](const GameObjectList::iterator findBegin, const int renderQueue)
		-> const GameObjectList::iterator
		{
			return std::lower_bound(
				findBegin, gameObjects.end(), renderQueue,
				[](const GameObjectPtr& e, const int value) { return e->renderQueue < value; });
		};	

	// stoneShadowキューの先頭はgameObjects.begin()と同じため検索しない
	
	// geometryキューの先頭を検索
	const auto& geometryBegin_itr =
		FindRenderQueueBegin(gameObjects.begin(), RenderQueue_geometry);

	// sprcularキューの先頭を検索
	const auto& sprcularBegin_itr =
		FindRenderQueueBegin(geometryBegin_itr, RenderQueue_sprcular);

	// effectキューの先頭を検索
	const auto& effectBegin_itr =
		FindRenderQueueBegin(sprcularBegin_itr, RenderQueue_effect);

	// transparentキューの先頭を検索
	const auto& transparentBegin_itr =
		FindRenderQueueBegin(effectBegin_itr, RenderQueue_transparent);

	// overlayキューの先頭を検索
	const auto& overlayBegin_itr =
		FindRenderQueueBegin(transparentBegin_itr, RenderQueue_overlay);

	// overlayキュー(UI)の優先度順にソート
	std::stable_sort(overlayBegin_itr, gameObjects.end(),
		[](const GameObjectPtr& a, const GameObjectPtr& b) {
			// それぞれのUIレイアウトを取得
			// レイアウトがなかったら後ろへ回す
			const auto& uiA = a->GetComponent<UILayout>();
			if (!uiA) return false;
			const auto& uiB = b->GetComponent<UILayout>();
			if (!uiB) return false;

			// 優先度が低いものを前へ移動
			return uiA->priority < uiB->priority; });

	// デプスシャドウマップを作成
	CreateShadowMap(gameObjects.begin(), effectBegin_itr);

	// 描画先をゲームウィンドウフレームバッファに戻す
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
	glViewport(0, 0, fbWidth, fbHeight);

	// ライト用SSBOをバインド
	lightBuffer->Bind();

	// stoneShadowキューを描画（不透明オブジェクト）
	DrawGameObject(*progDefaultLit_with_StoneShadow, *progUnlit, 
		0.5f, gameObjects.begin(), geometryBegin_itr);

	// geometryキューを描画（不透明オブジェクト）
	DrawGameObject(*progDefaultLit, *progUnlit, 
		0.5f, geometryBegin_itr, sprcularBegin_itr);

	// sprcularキューを描画（鏡面反射オブジェクト）
	DrawGameObject(*progSprcular, *progUnlit,
		0.5f, sprcularBegin_itr, effectBegin_itr);

	// ゲームウィンドウFBOにAOを適応
	if (saoParam.isEnabled) {
		DrawAmbientOcclusion();
	}

	// 描画先フレームバッファを戻す
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
	glViewport(0, 0, fbWidth, fbHeight);

	// スカイスフィアを描画
	DrawSkySphere(*progUnlit);

	// effectキューを描画
	glDepthMask(GL_FALSE);	// 深度バッファへの書き込みを禁止
	glDisable(GL_CULL_FACE);// 裏面カリングを無効化
	DrawGameObject(*progUnlit, *progUnlit, 
		0.0f, effectBegin_itr, transparentBegin_itr);
	glDepthMask(GL_TRUE);	// 深度バッファへの書き込みを許可
	glEnable(GL_CULL_FACE);	// 裏面カリングを有効化

	// パーティクルを描画
	particleBuffer->Draw();

	// ゲームウィンドウFBOにブルームエフェクトを適応
	if (screenBloomParam.isEnabled) {
		DrawBloomEffect(*fboMainGameWindow, screenBloomParam);
	}

	// 描画先フレームバッファを戻す
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
	glViewport(0, 0, fbWidth, fbHeight);

	// 深度テストを有効化
	glEnable(GL_DEPTH_TEST);	

	// 裏面カリングを無効化
	glDisable(GL_CULL_FACE);

	// 半透明合成を有効化
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// コライダーを可視化
	if (debugWindow && showCollision) {
		ShowCollision(*progUnlit);
	}

	// transparentキューを描画（半透明オブジェクト）
	glDepthMask(GL_FALSE); // 深度バッファへの書き込みを禁止
	DrawGameObject(*progDefaultLit, *progUnlit, 
		0.0f, transparentBegin_itr, overlayBegin_itr);
	glDepthMask(GL_TRUE); // 深度バッファへの書き込みを許可

	// overlay以降のキューを描画（UI）
	glDisable(GL_DEPTH_TEST); // 深度テストを無効化（無効化することでUIが壁に埋まらなくなる）
	DrawGameObject(*progUnlit, *progUnlit, 
		0.0f, overlayBegin_itr, gameObjects.end());

	// ライト用SSBOのバインドを解除
	lightBuffer->Unbind();


	// エミッションテクスチャFBOを作成
	if (emissionBloomParam.isEnabled)
	{	
		// 描画先をエミッションフレームバッファにする
		glBindFramebuffer(GL_FRAMEBUFFER, *fboEmission);
		// ビューポートをフレームバッファのサイズに合わせる
		const auto& texEmission = fboEmission->GetColorTexture();
		glViewport(0, 0, texEmission->GetWidth(), texEmission->GetHeight());
		
		// バックバッファを削除するときの色を指定
		glClearColor(0, 0, 0, 1);
		
		// バッファをクリア
		glClear(GL_COLOR_BUFFER_BIT);
		
		// 深度バッファへの書き込みを禁止(ゲームウィンドウと同じ深度テクスチャを使用するため)
		glDepthMask(GL_FALSE); 

		// 深度テストを有効化
		glEnable(GL_DEPTH_TEST);

		// 裏面カリングを有効化（裏側を描画しない）
		glEnable(GL_CULL_FACE);

		// フラグメントの深度値が深度バッファの値以下の場合に描画を許可
		glDepthFunc(GL_LEQUAL);
		
		// stoneShadow・geometry・sprcular・キューを描画（不透明オブジェクト）
		DrawEmissionGameObject(0.5f, gameObjects.begin(), effectBegin_itr);

		// effectキューを描画
		glDisable(GL_CULL_FACE);// 裏面カリングを無効化
		DrawEmissionGameObject(0.0f, effectBegin_itr, transparentBegin_itr);
		glEnable(GL_CULL_FACE);	// 裏面カリングを有効化

		DrawSkySphere(*progEmission);

		// パーティクルを描画
		particleBuffer->DrawEmission();

		// overlay以降のキューを描画（UI）
		glDisable(GL_DEPTH_TEST);	// 深度テストを無効化
		glDisable(GL_CULL_FACE);	// 裏面カリングを無効化
		DrawEmissionGameObject(0.0f, overlayBegin_itr, gameObjects.end());

		// エミッションFBOにブルームエフェクトを適応
		DrawBloomEffect(*fboEmission, emissionBloomParam);

		glDisable(GL_DEPTH_TEST);	// 深度テストを無効化
		glDisable(GL_CULL_FACE);	// 裏面カリングを無効化

		// エミッション(ブルーム)FBOをゲームウィンドウFBOに加算合成
		DrawTextureToFbo(*fboMainGameWindow, *progWindow,
			fboEmission->GetColorTexture(), GL_CLAMP_TO_EDGE);

		// 深度テスト設定を元の戻す
		glDepthFunc(GL_LESS);
	}

	// パーティクル用SSBOのバインドを解除
	particleBuffer->Unbind();

#if 0
	// マップのチェック用
	{
		// 描画先をゲームウィンドウフレームバッファに
		glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
		glUseProgram(*progUnlit);

		// 半透明合成を有効化
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(*meshBuffer->GetVAO());
		vec4 color = vec4(1);
		glProgramUniform4fv(*progUnlit, Loc::Color, 1, &color.x);
		MaterialList materials(1, std::make_shared<Material>());
		materials[0]->texBaseColor = fboSAOBlur->GetColorTexture();
		mat4 m = TransformMatrix(vec3(5, 1, 3), vec3(0), vec3(1));
		glProgramUniformMatrix4fv(*progUnlit, Loc::TransformMatrix, 1, GL_FALSE, &m[0].x);
		DrawStaticMesh(*meshBuffer->GetStaticMesh("Plane.obj"), *progUnlit, materials);
		glBindVertexArray(0);
	}
#endif

	// デフォルトフレームバッファに指定されているGBufferを描画する
	{
		// 描画先をデフォルトフレームバッファに
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, fbWidth, fbHeight);

		// アルファブレンドを有効に戻す
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// 深度テストを無効化
		glDisable(GL_DEPTH_TEST); 

		// VAOとシェーダをOpenGLコンテキストに割り当てる
		glUseProgram(*progWindow);
		glBindVertexArray(*meshBuffer->GetVAO());

		// マテリアルを作成
		Material mt;
		switch (renderGBuffer)
		{
		case GBuffer::MainGame:
			mt.texBaseColor = fboMainGameWindow->GetColorTexture();
			break;
		case GBuffer::Depth:
			mt.texBaseColor = fboMainGameWindow->GetDepthTexture();
			break;
		case GBuffer::Shadow:
			mt.texBaseColor = fboShadow->GetDepthTexture();
			break;
		case GBuffer::Emission:
			mt.texBaseColor = fboEmission->GetColorTexture();
			break;
		case GBuffer::SAO:
			mt.texBaseColor = fboSAOBlur->GetColorTexture();
			break;
		default:
			mt.texBaseColor = fboMainGameWindow->GetColorTexture();
			break;
		}

		// 作成したマテリアルをデフォルトフレームバッファに描画
		DrawSpritePlaneMesh(*windowMesh, *progWindow, vec4(1), mt);

		// VAOシェーダの割り当てを解除
		glBindVertexArray(0);
	}

	//フレームバッファの表示側と描画側を入れ替える
	glfwSwapBuffers(window);
}

/**
* デプスシャドウマップを作成
* 
* @param begin			描画するゲームオブジェクト範囲の先頭
* @param end			描画するゲームオブジェクト範囲の終端
*/
void Engine::CreateShadowMap(
	GameObjectList::iterator begin, GameObjectList::iterator end)
{
	// 描画先フレームバッファを変更
	glBindFramebuffer(GL_FRAMEBUFFER, *fboShadow);

	// 深度バッファをクリア
	glClear(GL_DEPTH_BUFFER_BIT);

	// ビューポートをフレームバッファのサイズに合わせる
	const auto& texShadow = fboShadow->GetDepthTexture();
	glViewport(0, 0, texShadow->GetWidth(), texShadow->GetHeight());

	// VAOとシェーダをOpenGLコンテキストに割り当てる
	glBindVertexArray(*meshBuffer->GetVAO());
	glUseProgram(*progShadow);

	// 影の描画パラメータ
	const float shadowAreaSize = 100; // 影の描画範囲(XY平面)
	const float shadowNearZ = 1;      // 影の描画範囲(近Z平面)
	const float shadowFarZ = 200;     // 影の描画範囲(遠Z平面)
	const float shadowCenterZ = (shadowNearZ + shadowFarZ) * 0.5f; // 描画範囲の中心
	const vec3 target = { 0, 0, 0 }; // カメラの注視点
	vec3 eye;
	if (directionalLight) {	/* 平行光源の有無は事前に確認済みのためここでは削除チェックはしない */
		eye = target - directionalLight->GetDirection() * shadowCenterZ; // カメラの位置
	}
	else {
		eye = target - vec3(0) * shadowCenterZ;
	}

	// ビュープロジェクション行列を計算
	const mat4 matShadowView = LookAt(eye, target, vec3(0, 1, 0));
	const mat4 matShadowProj = Orthogonal(
		-shadowAreaSize / 2, shadowAreaSize / 2,
		-shadowAreaSize / 2, shadowAreaSize / 2,
		shadowNearZ, shadowFarZ);
	const mat4 matShadow = matShadowProj * matShadowView;

	// ビュープロジェクション行列をGPUメモリにコピー
	glProgramUniformMatrix4fv(*progShadow,
		Loc::ViewProjectionMatrix, 1, GL_FALSE, &matShadow[0].x);
	glProgramUniformMatrix4fv(*progShadowAnimation,
		Loc::ViewProjectionMatrix, 1, GL_FALSE, &matShadow[0].x);

	// メッシュを描画
	for (GameObjectList::iterator i = begin; i != end; ++i) {
		const auto& gObj = *i;
		if (!gObj->CheckActive()) {
			// アクティブではない
			continue;
		}

		for (const auto& r : gObj->renderers) {
			if (!r->isActive) {
				// アクティブではない
				continue;
			}
			if (!r->castShadow) {
				// 影を落とさない
				continue;
			}

			//頂点アトリビュートをバインド
			glBindVertexArray(*meshBuffer->GetVAO());

			switch (r->GetModelFormat())
			{
				// OBJ・glTF(アニメーションなし)を描画
			case ModelFormat::obj:
			case ModelFormat::gltfStatic:
				r->Draw(*progShadow);
				break;

				// glTF(アニメーションあり)を描画
			case ModelFormat::gltfAnimated:
				glUseProgram(*progShadowAnimation);
				r->Draw(*progShadowAnimation);
				glUseProgram(*progShadow);
				break;
			}
		}
	} // for

	// 深度テクスチャを割り当てる
	glBindTextureUnit(Bind::Texture::Shadow, *texShadow);

	// 深度テクスチャ座標への変換行列を作成
	static const mat4 matTexture = {
		{ 0.5f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.5f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.5f, 0.0f },
		{ 0.5f, 0.5f, 0.5f, 1.0f }
	};

	// NDC座標系からテクスチャ座標系へ変換
	const mat4 matShadowTexture = matTexture * matShadow * matShadowView;

	// 法線方向の補正値を設定
	const float texelSize = shadowAreaSize / static_cast<float>(texShadow->GetWidth());

	for (const auto& prog : affectedLightPrograms) {
		// シャドウテクスチャ行列
		glProgramUniformMatrix4fv(prog,
			Loc::ShadowTextureMatrix, 1, GL_FALSE, &matShadowTexture[0].x);

		// 法線補正値
		glProgramUniform1f(prog, Loc::ShadowNormalOffset, texelSize);
	}
}

/**
* ゲームオブジェクト配列を描画する
*
* @param progLit		光の影響を受けるオブジェクトの描画に使うプログラム
* @param progUnlit		光の影響を受けないオブジェクトの描画に使うプログラム
* @param alphaCutoff	フラグメントを破棄する境界値
* 
* @param begin			描画するゲームオブジェクト範囲の先頭
* @param end			描画するゲームオブジェクト範囲の終端
*/
void Engine::DrawGameObject(
	ProgramPipeline& progLit, ProgramPipeline& progUnlit, float alphaCutoff,
	GameObjectList::iterator begin, GameObjectList::iterator end)
{
	for (GameObjectList::iterator itr = begin; itr != end; ++itr) {
		const auto& gObj = *itr;
		if (!gObj->CheckActive()) {
			// アクティブではない
			continue;
		}

		// ゲームオブジェクトの持っている全てのメッシュを描画
		for (const auto& r : gObj->renderers) {
			if (!r->isActive) {
				continue;
			}

			// モデルフォーマットと光の影響の有無によってプログラムを選択
			ProgramPipeline* prog = nullptr;
			switch (r->GetModelFormat())
			{
				// OBJ・glTF(アニメーションなし)を描画
			case ModelFormat::obj:
			case ModelFormat::gltfStatic:
				prog = &(r->useLighting ? progLit : progUnlit);
				break;

				// glTF(アニメーションあり)を描画
			case ModelFormat::gltfAnimated:
				prog = &(r->useLighting ? *progAnimationLit : *progAnimationUnlit);
				break;

			default:
				continue;
			}

			glUseProgram(*prog);

			//頂点アトリビュートをバインド
			glBindVertexArray(*meshBuffer->GetVAO());

			// フラグメントを破棄する境界値を設定
			glProgramUniform1f(*prog, Loc::AlphaCutoff, alphaCutoff);

			r->Draw(*prog);
		}
	}
}

/**
* スカイスフィアを描画する
* 
* @param prog	描画に使うプログラム
*/
void Engine::DrawSkySphere(ProgramPipeline& prog)
{
	// シーンにスカイスフィアが設定されていない場合は描画しない
	if (!skySphere || !scene->skysphereMaterial) {
		return;
	}

	glDepthMask(GL_FALSE); // 深度バッファへの書き込みを禁止

	// アンリットシェーダで描画
	glUseProgram(prog);
	glBindVertexArray(*meshBuffer->GetVAO());

	// スカイスフィアモデルの半径を0.5mと仮定し、描画範囲の95%の大きさに拡大
	static const float far = 1000; // 描画範囲の最大値
	static const float scale = far * 0.95f / 0.5f;
	static const mat4 transformMatrix = {
		{ scale, 0,    0,  0 },
		{ 0, scale,    0,  0 },
		{ 0,     0, scale, 0 },
		{ 0,     0,    0,  1 },
	};
	glProgramUniformMatrix4fv(prog,
		Loc::TransformMatrix, 1, GL_FALSE, &transformMatrix[0].x);

	glProgramUniform1f(prog, Loc::AlphaCutoff, 1);

	bool hasUniformColor = 	// カラーユニフォームの有無
		glGetUniformLocation(prog, "color") >= 0;
	if (hasUniformColor) {
		// 色はマテリアルカラーで調整するのでオブジェクトカラーは白に設定
		static const vec4 color(1);
		glProgramUniform4fv(prog, Loc::Color, 1, &color.x);
	}

	// スカイスフィアを描画
	const MaterialList materials(1, scene->skysphereMaterial);
	DrawStaticMesh(*skySphere, prog, vec4(1), materials);
	
	glProgramUniform1f(prog, Loc::AlphaCutoff, 0);
	glDepthMask(GL_TRUE); // 深度バッファへの書き込みを許可
}

/**
* ゲームオブジェクト配列を描画する
*
* @param alphaCutoff	フラグメントを破棄する境界値
*
* @param begin			描画するゲームオブジェクト範囲の先頭
* @param end			描画するゲームオブジェクト範囲の終端
*/
void Engine::DrawEmissionGameObject(
	float alphaCutoff,
	GameObjectList::iterator begin, GameObjectList::iterator end)
{
	// フラグメントを破棄する境界値を設定
	glProgramUniform1f(*progEmission, 
		Loc::AlphaCutoff, alphaCutoff);
	glProgramUniform1f(*progEmissionAnimation,
		Loc::AlphaCutoff, alphaCutoff);

	glUseProgram(*progEmission);

	for (GameObjectList::iterator itr = begin; itr != end; ++itr) {
		const auto& gObj = *itr;
		if (!gObj->CheckActive()) {
			// アクティブではない
			continue;
		}

		for (const auto& r : gObj->renderers) {
			if (!r->isActive) {
				// アクティブではない
				continue;
			}

			//頂点アトリビュートをバインド
			glBindVertexArray(*meshBuffer->GetVAO());

			switch (r->GetModelFormat())
			{
				// OBJ・glTF(アニメーションなし)を描画
			case ModelFormat::obj:
			case ModelFormat::gltfStatic:
				r->Draw(*progEmission);
				break;

				// glTF(アニメーションあり)を描画
			case ModelFormat::gltfAnimated:
				glUseProgram(*progEmissionAnimation);
				r->Draw(*progEmissionAnimation);
				glUseProgram(*progEmission);
				break;
			}
		}
	}
}


/**
* ブルームを描画
* 
* @param fbo		ブルームを適応するFBO
* @param param		ブルームパラメータ
*/
void Engine::DrawBloomEffect(
	FramebufferObject& fbo, const BloomParam& param)
{
	// ブルームエフェクト用にGLコンテキストを設定
	glDisable(GL_DEPTH_TEST);	// 深度テストを無効化
	glDisable(GL_CULL_FACE);	// 裏面カリングを無効化

	// 1. ゲーム画面用FBOから高輝度成分を抽出
	glDisable(GL_BLEND); // 合成なし(半透明合成を行わないため)
	{
		// 高輝度成分はFBOの枚数だけ加算されるので、その影響を打ち消すために枚数で割る
			/*加算合計の結果が1倍になるように、ブルームの強さをFBOの枚数で除算*/
		const float s = param.strength / static_cast<float>(std::size(fboBloom));
		glProgramUniform2f(
			*progHighPassFilter, Loc::Bloom::BloomInfo, param.threshold, s);

		DrawTextureToFbo(*fboBloom[0], *progHighPassFilter,
			fbo.GetColorTexture(), GL_CLAMP_TO_BORDER);
	}

	// 2. 縮小ぼかしを行い、1段小さいFBOにコピー
	for (size_t i = 1; i < fboBloom.size(); ++i) { /*fboBloom[0] ～ fboBloom[5]*/
		DrawTextureToFbo(*fboBloom[i], *progDownSampling,
			fboBloom[i - 1]->GetColorTexture(), GL_CLAMP_TO_BORDER);
	}

	// 3. 拡大ぼかしを行い、1段大きいFBOに加算合成
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE); // 加算合成（結果の色 = シェーダ出力 * 1.0(GL_ONE) + FBO * 1.0(GL_ONE) = シェーダ出力 + FBO）
	for (size_t i = fboBloom.size() - 1; i > 0; ) {
		--i;
		DrawTextureToFbo(*fboBloom[i], *progUpSampling,
			fboBloom[i + 1]->GetColorTexture(), GL_CLAMP_TO_EDGE);
	}

	// 4. 最後の拡大ぼかしを行い、ゲーム画面用FBOに加算合成
	{
		DrawTextureToFbo(fbo, *progWindow,
			fboBloom[0]->GetColorTexture(), GL_CLAMP_TO_EDGE);	/*ゲーム画面用FBO*/
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

/**
* アンビエントオクルージョンを描画
*/
void Engine::DrawAmbientOcclusion()
{
	// カリング、深度テスト、アルファブレンドを無効化
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	// カメラパラメータを取得
	float near, far;
	vec2 fovScale;
	if (!freeCamera) {
		Camera* cameraComp = GetMainCamera_CameraComponent();
		near = cameraComp->near;
		far = cameraComp->far;
		fovScale = cameraComp->fovScale;
	}
	else {
		near = freeCamera->near;
		far = freeCamera->far;
		fovScale = freeCamera->fovScale;
	}

	// 1. 深度値を線形距離に変換
	{
		// カメラの近・遠平面値をユニフォーム変数にコピー
		glProgramUniform2f(*progSAORestoreDepth, Loc::Sao::CameraNearFar,
			near, far);

		DrawTextureToFbo(*fboSAODepth[0], *progSAORestoreDepth,
			fboMainGameWindow->GetDepthTexture(), GL_CLAMP_TO_BORDER);
	}

	// 2. 縮小バッファを作成
	{
		for (int i = 1; i < fboSAODepth.size(); ++i) {
			glProgramUniform1i(*progSAODownSampling, Loc::Sao::MipLevel, i - 1);
			DrawTextureToFbo(*fboSAODepth[i], *progSAODownSampling,
				fboSAODepth[0]->GetColorTexture(), GL_CLAMP_TO_BORDER);
		}
	}

	// 3. SAOを計算
	{
		// SAO制御用パラメータを設定
		glProgramUniform4f(*progSAOCalc, Loc::Sao::SaoInfo,
			saoParam.radius * saoParam.radius,
			0.5f * saoParam.radius * fovScale.y,
			saoParam.bias,
			saoParam.intensity / pow(saoParam.radius, 6.0f)
		);

		// NDC座標を視点座標に変換するパラメータを設定
		const float w = static_cast<float>(fboSAODepth[0]->GetWidth());
		const float h = static_cast<float>(fboSAODepth[0]->GetHeight());
		const float aspectRatio = w / h;
		const float invFovScale = 1.0f / fovScale.y;
		glProgramUniform2f(*progSAOCalc, Loc::Sao::NdcToView,
			invFovScale * aspectRatio, invFovScale);
	
		DrawTextureToFbo(*fboSAOCalc, *progSAOCalc,
			fboSAODepth[0]->GetColorTexture(), GL_CLAMP_TO_BORDER);
	}

	// 4. SAOの結果をぼかす
	{
		glUseProgram(*progSAOBlur);
		glBindFramebuffer(GL_FRAMEBUFFER, *fboSAOBlur);

		// 遮蔽率テクスチャ
		GLuint texColor = *fboSAOCalc->GetColorTexture();
		glBindTextures(0, 1, &texColor);
		
		// 線形深度値
		GLuint texDepth = *fboSAODepth[0]->GetColorTexture();
		glBindTextures(1, 1, &texDepth);

		glBindVertexArray(*meshBuffer->GetVAO());
		for (const auto& prim : windowMesh->primitives) {
			DrawPrimitive(prim);
		}
	}

	// 5. アンビエントオクルージョンをゲームウィンドウに合成
	{
		glUseProgram(*progSAO);
		glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
		glViewport(0, 0, fboMainGameWindow->GetWidth(), fboMainGameWindow->GetHeight());

		// SAOテクスチャ
		GLuint texShield = *fboSAOBlur->GetColorTexture();
		glBindTextures(0, 1, &texShield);
		
		// ゲームウィンドウテクスチャ
		GLuint texColor = *fboMainGameWindow->GetColorTexture();
		glBindTextures(1, 1, &texColor);

		glBindVertexArray(*meshBuffer->GetVAO());
		for (const auto& prim : windowMesh->primitives) {
			DrawPrimitive(prim);
		}
	}

	// カリング、深度テスト、アルファブレンドを有効化
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}



/**
* コリジョンを表示する
*/
void Engine::ShowCollision(ProgramPipeline& prog)
{
	glUseProgram(prog);
	glBindVertexArray(*meshBuffer->GetVAO());

	const GLuint tex = *TextureManager::GetTexture("White.tga");
	glBindTextures(Bind::Texture::Color, 1, &tex);
	glBindTextures(Bind::Texture::Emission, 1, nullptr);

	const vec4 white = vec4(1);
	for (const auto& gObj : gameObjects) {
		if (!gObj->CheckActive()) {
			continue;
		}
		if (gObj->colliders.empty()) {
			continue;
		}
		for (const auto& col : gObj->colliders) {
			glProgramUniform4fv(prog, Loc::Color, 1, &white.x);
			col->DrawCollision(prog);
		}
	}

	glBindVertexArray(0);
}

/**
* ImGuiによる情報表示
*/
void Engine::RenderImGui()
{
	glfwMakeContextCurrent(debugWindow);

	// バックバッファを削除
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// システム
	{
		ImGui::Begin("System", NULL, ImGuiWindowFlags_NoMove);
		ImGui::Text("FPS: %f", fps);	// FPS表示
		ImGui::Text("DeltaTime: %f", deltaSeconds);	// 前回更新からの経過時間表示

		{
			ImGui::SeparatorText("");

			// フリーカメラの使用有無
			bool useFreeCamera = (freeCamera != nullptr);
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImGui::Checkbox("FreeCamera", &useFreeCamera);
			ImGui::PopStyleColor();
			if (useFreeCamera) {
				// フリーカメラがまだ作成されていなかったら作成
				if (!freeCamera) {
					// 現在のメインカメラの位置を取得
					const auto& cameraObj = GetMainCameraObject();
					vec3 pos, rot, sca;
					Decompose(cameraObj->GetWorldTransformMatrix(), pos, rot, sca);		
					
					freeCamera = new FreeRoamCamera(pos, rot);

					// インプットシステムを無効化
					InputSystem::GetInstance().isEnabled = false;
				}
			}
			else {
				// フリーカメラが残っていたら削除
				if (freeCamera) {
					delete freeCamera;
					freeCamera = nullptr;

					// インプットシステムを有効化
					InputSystem::GetInstance().isEnabled = true;
				}
			}

			// ゲームスピード
			ImGui::DragFloat("GameSpeed", &gameSpeed);
			gameSpeed = std::max(gameSpeed, 0.0f);
			if (ImGui::Button("Reset")) gameSpeed = 1.0f;
			ImGui::SameLine();
			if (ImGui::Button("Stop"))	gameSpeed = 0.0f;
			ImGui::Spacing();

			// タイムスケール
			ImGui::DragFloat("TimeScale", &timeScale);
			timeScale = std::max(timeScale, 0.0f);
			ImGui::Spacing();

			// コリジョン表示有無・表示色設定
			ImGui::Checkbox("ShowCollision", &showCollision);
			if (ImGui::TreeNode("CollisionColor")) {
				ImGui::ColorEdit4("General",	&collisionColor[0].x);
				ImGui::ColorEdit4("Deactive",	&collisionColor[1].x);
				ImGui::ColorEdit4("Trigger",	&collisionColor[2].x);
				ImGui::ColorEdit4("Static",		&collisionColor[3].x);

				ImGui::TreePop();
			}
			ImGui::Spacing();

			// 環境光
			ImGui::Text("AmbientLight");
			ImGui::ColorEdit3("Color", &ambientLight.x);
			ImGui::Spacing();

			// シーン
			if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
				// 現在シーンの情報
				if (ImGui::TreeNode("CurrentScene Info")) {
					scene->DrawImGui();
					ImGui::TreePop();
				}
				ImGui::Spacing();

				// シーン還元ボタン作成マクロ
				#define CREATE_SCENE_CHANGE_BUTTON(sceneName, sceneClass) \
					do { \
						if (ImGui::Button(sceneName)) { \
							SetNextScene<sceneClass>(); \
							EasyAudio::Stop(AudioPlayer::bgm); \
						} \
				    } while (0)    

				// 各シーンの還元ボタン作成
				CREATE_SCENE_CHANGE_BUTTON("Debug", DebugSnene);
				CREATE_SCENE_CHANGE_BUTTON("Title", TitleScene);
				CREATE_SCENE_CHANGE_BUTTON("LoadMainGame", LoadMainGameScene);
				CREATE_SCENE_CHANGE_BUTTON("MainGame", MainGameScene);
				CREATE_SCENE_CHANGE_BUTTON("BackToTitle", BackToTitleScene);

				ImGui::TreePop();
			}

			// GBufferの表示切り替え
			if (ImGui::TreeNodeEx("Render GBuffer", ImGuiTreeNodeFlags_DefaultOpen)) {
				int GBufferNum = renderGBuffer;
				ImGui::RadioButton("MainGame", &GBufferNum, GBuffer::MainGame);
				ImGui::RadioButton("Depth", &GBufferNum, GBuffer::Depth);
				ImGui::RadioButton("Shadow", &GBufferNum, GBuffer::Shadow);
				ImGui::RadioButton("Emission", &GBufferNum, GBuffer::Emission);
				ImGui::RadioButton("SAO", &GBufferNum, GBuffer::SAO);
				renderGBuffer = static_cast<GBuffer>(GBufferNum);

				ImGui::TreePop();
			}

		} {
			// ゲームオブジェクト数表示
			ImGui::SeparatorText("Information");
			ImGui::Text("GameObjects:     %d", static_cast<int>(gameObjects.size()));
			
			// タグ持ちゲームオブジェクト数表示
			int tagCnt = 0;
			for (const auto& pair : gameObjects_tag) {
				tagCnt += static_cast<int>(pair.second.size());
			}
			ImGui::Text("GameObjectTags:  %d", tagCnt);
			// 各タグごとに設定されているゲームオブジェクト数表示
			if (ImGui::TreeNode("Tag Info")) {
				for (const auto& pair : gameObjects_tag) {
					std::string name = pair.first.GetName();
					if (name.size() < 16) {
						name.resize(16, ' '); // 足りない部分を空白で埋める
					}
					ImGui::Text(
						"%s [%d]", name.c_str(), static_cast<int>(pair.second.size()));
				}
				ImGui::TreePop();
			}
		}

		{
			ImGui::SeparatorText("PostProcess");
			if (ImGui::TreeNode("ScreenBloom")) {
				ImGui::Checkbox("Enable", &screenBloomParam.isEnabled);
				ImGui::DragFloat("Strength", &screenBloomParam.strength);
				ImGui::DragFloat("Threshold", &screenBloomParam.threshold);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("EmissionBloom")) {
				ImGui::Checkbox("Enable", &emissionBloomParam.isEnabled);
				ImGui::DragFloat("Strength", &emissionBloomParam.strength);
				ImGui::DragFloat("Threshold", &emissionBloomParam.threshold);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("SAO")) {
				ImGui::Checkbox("Enable", &saoParam.isEnabled);
				ImGui::DragFloat("Radius", &saoParam.radius);
				ImGui::DragFloat("Bias", &saoParam.bias);
				ImGui::DragFloat("Intensity", &saoParam.intensity);
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	// ゲームオブジェクト
	{
		ImGui::Begin("GameObject", NULL, ImGuiWindowFlags_NoMove);

		// ゲームオブジェクトの親子関係を考慮して表示する関数
		std::function<void(GameObject*)> GameObject_DrawImGui =
			[this, &GameObject_DrawImGui](GameObject* obj)
			{
				ImGuiTreeNodeFlags node_flags =
					ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_OpenOnDoubleClick |
					ImGuiTreeNodeFlags_SpanAvailWidth;

				// 選択しているゲームオブジェクトか
				if (obj == selectingGameObjct.lock().get()) {
					node_flags |= ImGuiTreeNodeFlags_Selected;
				}

				// 子オブジェクトを持っているか
				if (obj->GetChildCount() == 0) {
					node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					ImGui::TreeNodeEx(
						static_cast<void*>(obj), node_flags, obj->name.GetName().c_str());

					// 選択されているか
					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
						// ゲームオブジェクトポインタを配列から探し,
						// 選択オブジェクトに設定する
						auto itr = std::find_if(gameObjects.begin(), gameObjects.end(),
							[&obj](const GameObjectPtr& p) { return p.get() == obj; });

						/*必ず見つかるためnullチャックはしない*/
						selectingGameObjct = *itr;
					}
				}
				else {
					bool openTreeNode =
						ImGui::TreeNodeEx(
							static_cast<void*>(obj), node_flags, obj->name.GetName().c_str());

					// 選択されているか
					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
						// ゲームオブジェクトポインタを配列から探し,
						// 選択オブジェクトに設定する
						const auto& itr = std::find_if(gameObjects.begin(), gameObjects.end(),
							[&obj](const GameObjectPtr& p) { return p.get() == obj; });

						/*必ず見つかるためnullチャックはしない*/
						selectingGameObjct = *itr;
					}

					if (openTreeNode) {
						// ノードが開いていたら子供も表示する
						for (const auto& c : obj->children) {
							GameObject_DrawImGui(c);
						}
						ImGui::TreePop();
					}
				}
			};

		//全てのゲームオブジェクトのパラメーターを表示
		for (const auto& obj : gameObjects) {
			if (obj->GetParent()) {
				// 親が設定されていたら飛ばす
				continue;
			}

			// ゲームオブジェクトを表示する
			GameObject_DrawImGui(obj.get());
		}
		ImGui::End();
	}

	// インスペクター
	{
		ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_NoMove);
		if (const auto& sobj = selectingGameObjct.lock()) {
			sobj->DrawImGui();
		}
		ImGui::End();
	}

	// ImGuiDemoを表示
	{
		//ImGui::ShowDemoWindow();
	}

	// ログを表示
	{
		LogBuffer::GetInstance().DrawImGui_AllLogs();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(debugWindow);
}
