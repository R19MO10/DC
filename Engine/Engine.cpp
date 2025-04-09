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
* OpenGL����̃��b�Z�[�W����������R�[���o�b�N�֐�
*
* @param source    ���b�Z�[�W�̔��M��(OpenGL�AWindows�A�V�F�[�_�Ȃ�)
* @param type      ���b�Z�[�W�̎��(�G���[�A�x���Ȃ�)
* @param id        ���b�Z�[�W����ʂɎ��ʂ���l
* @param severity  ���b�Z�[�W�̏d�v�x(���A���A��A�Œ�)
* @param length    ���b�Z�[�W�̕�����. �����Ȃ烁�b�Z�[�W��0�I�[����Ă���
* @param message   ���b�Z�[�W�{��
* @param userParam �R�[���o�b�N�ݒ莞�Ɏw�肵���|�C���^
*/
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (type != GL_DEBUG_TYPE_ERROR) {	//�G���[�ȊO�Ȃ�s��Ȃ�
		return;
	}

	std::string s;
	if (length < 0) {
		//���b�Z�[�W�͏I�[(�I��肪�ۏႳ��Ă���)�̂ł��̂܂ܑ������
		s = message;
	}
	else {
		//�n�܂�(message)����I���(message + length)���������
		s.assign(message, message + length);
	}
	s += '\n'; // ���b�Z�[�W�ɂ͉��s���Ȃ��̂Œǉ�����
	//��������o�͂���
	LOG(s.c_str());
}


/**
* �Q�[���G���W�������s����
*
* @retval 0     ����Ɏ��s����������
* @retval 0�ȊO �G���[����������
*/
int Engine::Run()
{
	{
		const int result = Initialize();
		if (result) {
			return result;
		}
	}

	// �������C�u������������
	if (!EasyAudio::Initialize()) {
		return 1; // ���������s
	}
	
	float changeScreenModeInterval = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		// �����I��
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			if (!debugWindow && !showMouseCursor) {
				// �J�[�\�����\������
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}


			int ms = MessageBox(NULL, "Do you want to exit the game?",
				"Execution Confirmation", MB_ICONEXCLAMATION | MB_OKCANCEL);
			if (ms == IDOK) {
				break;
			}
			else {
				if (!debugWindow && !showMouseCursor) {
					// �J�[�\�����\���ɂ���
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
			}
		}

		// �X�N���[�����[�h�̐؂�ւ�
		if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && 
			changeScreenModeInterval <= 0) 
		{
			if (isFullScreen) {
				// �E�B���h�E���[�h�֕ύX
				glfwSetWindowMonitor(window, nullptr, 100, 100, 1280, 720, 60);
				isFullScreen = false;
			}
			else {
				// �t���X�N���[���֕ύX
				GLFWmonitor* monitor = glfwGetPrimaryMonitor();	// ���j�^�[���擾
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);	// �r�f�I���[�h���擾
				glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

				MatchFboSize();	// FBO���쐬���Ȃ���
				isFullScreen = true;
			}

			changeScreenModeInterval = 1.0f;
		}

		// �f�o�b�N�E�B���h�E�\���E��\���̐؂�ւ�
		if (!debugWindow) {
			// �f�o�b�N�E�B���h�E�����Ă�����
			if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
				// �J�[�\����\���ɂ���
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

				// �f�o�b�N�E�B���h�E���쐬����
				glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
				debugWindow = 
					glfwCreateWindow(1000, 750, debugTitle.c_str(), nullptr, nullptr);

				if (debugWindow) {
					// �A�C�R���摜��ǂݍ���ŃA�C�R���ɐݒ�
					const GLFWimage& iconImage =
						LoadIcon_as_GLFWimage("Res/Icon/DebugWindow.ico");
					if (iconImage.pixels) {
						glfwSetWindowIcon(debugWindow, 1, &iconImage);
					}

					// ImGui�̏�����
					ImGui::CreateContext();
					ImGui_ImplGlfw_InitForOpenGL(debugWindow, true);
					const char* glsl_version = "#version 450";
					ImGui_ImplOpenGL3_Init(glsl_version);
				}
			}
		}
		else {
			// �f�o�b�N�E�B���h�E���J���Ă�����

			// �����I��
			if (glfwGetKey(debugWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				break;
			}

			// �f�o�b�N�E�B���h�E�����
			if (glfwWindowShouldClose(debugWindow)) {
				// �J�[�\���������ɖ߂�
				if (!showMouseCursor) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}

				// �f�o�b�N�E�B���h�E���I������
				glfwDestroyWindow(debugWindow);
				debugWindow = nullptr;

				// ImGui�̏I��
				ImGui_ImplOpenGL3_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();
			}
		}

		glfwMakeContextCurrent(window);

		// FPS���v��
		const double curTime_s = glfwGetTime();

		const double diffTime_s = curTime_s - fpsTime;
		if (diffTime_s >= 1) {
			// 1�b���Ƃ�FPS���v�Z
			fps = static_cast<float>(fpsCount / diffTime_s);

			// �v���p�ϐ������Z�b�g
			fpsTime = curTime_s;
			fpsCount = 0;
		}

		// �f���^�^�C��(�O��̍X�V����̌o�ߎ���)���v�Z
		deltaSeconds = static_cast<float>(curTime_s - previousTime_s);

		// �o�ߎ��Ԃ���������ꍇ�͓K���ɒZ������(��Ƀf�o�b�O�΍�)
		if (deltaSeconds >= 0.5f) {
			deltaSeconds = oneFpsTime_s;
		}

		if (deltaSeconds >= oneFpsTime_s) {	// �X�V�Ԋu
			previousTime_s = curTime_s;
			++fpsCount;

			Update();

			if (mainCamera) {
				Render();
			}
		}

		// �������C�u�������X�V
		EasyAudio::Update();

		if (debugWindow) {
			RenderImGui();
		}

		// �t���X�N���[���C���^�[�o��
		if (changeScreenModeInterval > 0) {
			changeScreenModeInterval -= deltaSeconds;
		}

		//OS����̗v��������(�}�E�X��L�[�{�[�h�Ȃǂ̏�Ԃ��擾)
		glfwPollEvents();
	}

	// �t���[�J�������폜
	if (freeCamera) {
		delete freeCamera;
	}

	// �������C�u�������I��
	EasyAudio::Finalize();

	// GLFW�̏I��
	glfwTerminate();	//�E�B���h�E���I������ەK���Ăяo��

	return 0;
}

/**
* �R���C�_�[�̏󋵂ɉ������F���擾����
*/
vec4 Engine::GetCollisionColor(const Collider& collider)
{
	if (!collider.isActive) return collisionColor[1];
	if (collider.isTrigger) return collisionColor[2];
	if (collider.isStatic) return collisionColor[3];
	return collisionColor[0];
}

/**
* UI�I�u�W�F�N�g���쐬����
*
* @param name		UI�I�u�W�F�N�g�̖��O
* @param filename	UI�I�u�W�F�N�g�ɕ\������摜
* @param magnification    UI�I�u�W�F�N�g�̑傫��
*
* @return �쐬����UI�I�u�W�F�N�g
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

	// �摜�T�C�Y�ɉ����Ċg�嗦�𒲐�
	const float aspectRatio = 
		uiObj.spriteRender->material.texBaseColor->GetAspectRatio();
	uiObj.spriteRender->scale = { magnification * aspectRatio, magnification, 1 };

	// �R���|�[�l���g��ǉ�
	uiObj.uiLayot = uiObj.object->AddComponent<UILayout>();

	return uiObj;
}

/**
* �t���[���o�b�t�@�̑傫�����擾����
*
* @return �t���[���o�b�t�@�̏c�Ɖ��̃T�C�Y
*/
vec2Int Engine::GetFlamebufferSize() const
{
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	return { w, h };
}

/**
* �t���[���o�b�t�@�̃A�X�y�N�g����擾����
*
* @return �t���[���o�b�t�@�̃A�X�y�N�g��
*/
float Engine::GetAspectRatio() const
{
	const vec2 size = vec2(GetFlamebufferSize());
	return size.x / size.y;
}

/**
* �X�^�e�B�b�N���b�V���̎擾
*
* @param name ���b�V����
*
* @return ���O��name�ƈ�v����X�^�e�B�b�N���b�V��
*/
StaticMeshPtr Engine::GetStaticMesh(const char* name)
{
	return meshBuffer->GetStaticMesh(name);
}


/**
* glTF�t�@�C���̎擾
*
* @param name glTf�t�@�C���̖��O
*
* @return ���O��name�ƈ�v����glTf�t�@�C��
*/
GltfFilePtr Engine::GetGltfFile(const char* name)
{
	return gltfFileBuffer->GetGltfFile(name);
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^��ǉ�
*
* @param matBones SSBO�ɒǉ�����p���s��̔z��
+*
* @return matBones�p�Ɋ��蓖�Ă�ꂽSSBO�͈̔�
*/
GltfFileRange Engine::AddAnimationMatrices(const AnimationMatrices& matBones)
{
	return gltfFileBuffer->AddAnimationMatrices(matBones);
}

/**
* �A�j���[�V�������b�V���̕`��Ɏg��SSBO�̈�����蓖�Ă�
*
* @param bindingPoint �o�C���f�B���O�|�C���g
* @param range        �o�C���h����͈�
*/
void Engine::BindAnimationBuffer(GLuint bindingPoint, const  GltfFileRange& range)
{
	gltfFileBuffer->BindAnimationBuffer(bindingPoint, range);
}


/**
* �V�������C�g���擾����
*
* @param lightPtr ���C�g�R���|�[�l���g�|�C���^
*/
void Engine::AllocateLight(const LightPtr& lightPtr)
{
	lightBuffer->Allocate(lightPtr);
}


/**
* �G�~�b�^�[��ǉ�����
*
* @param  emitterParam  �G�~�b�^�[�̏������p�����[�^
*
* @return �ǉ������G�~�b�^�[
*/
ParticleEmitterPtr Engine::AddParticleEmitter(const ParticleEmitterParameter& emitterParam)
{
	return particleBuffer->AddEmitter(emitterParam);
}

/**
* �w�肳�ꂽ���O�����G�~�b�^�[����������
*
* @param name  ��������ID
*
* @return ����id�ƈ�v����ID�����G�~�b�^�[
*/
ParticleEmitterPtr Engine::FindParticleEmitter(const char* name) const
{
	return particleBuffer->FindEmitter(name);
}

/**
* �w�肳�ꂽ�G�~�b�^�[���폜����
*/
void Engine::RemoveParticleEmitter(const ParticleEmitterPtr& emitterPtr)
{
	particleBuffer->RemoveEmitter(emitterPtr);
}

/**
* ���ׂẴG�~�b�^�[���폜����
*/
void Engine::RemoveParticleEmitterAll()
{
	particleBuffer->RemoveEmitterAll();
}


/**
* FBO�̃T�C�Y���t���[���o�b�t�@�̃T�C�Y�ɍ��킹��
*/
void Engine::MatchFboSize()
{
	// �t���X�N���[���Ȃ�ω����邱�Ƃ͂Ȃ����ߏ������Ȃ�
	if (isFullScreen) {
		return;
	}

	// �r���[�|�[�g���t���[���o�b�t�@�̃T�C�Y�ɍ��킹��
	const auto& texGameWindow = fboMainGameWindow->GetColorTexture();
	const vec2Int fboFrame = {
		texGameWindow->GetWidth(),
		texGameWindow->GetHeight()
	};

	const vec2Int currentFlame = GetFlamebufferSize();

	// �t���[���o�b�t�@��FBO�̑傫�����قȂ�ꍇ��FBO����蒼��
	if (currentFlame != fboFrame) {

		// �V�F�A�[�h�|�C���^���㏑�����邱�Ƃ�FBO��j������

		// �Q�[���E�B���h�E�pFBO���쐬
		const auto& texGameWindowColor = TextureManager::CreateTexture(
			"FBO(color)", currentFlame.x, currentFlame.y, GL_RGBA16F);
		const auto& texGameWindowDepth = TextureManager::CreateTexture(
			"FBO(depth)", currentFlame.x, currentFlame.y, GL_DEPTH_COMPONENT32F);
		fboMainGameWindow = FramebufferObject::Create(texGameWindowColor, texGameWindowDepth);

		// �G�~�b�V�����pFBO���쐬
		const auto& texEmissionColor = TextureManager::CreateTexture(
			"FBO(color)", currentFlame.x, currentFlame.y, GL_RGBA16F);
		/* �Q�[���E�B���h�E�Ɠ����[�x�e�N�X�`�����g�p */
		fboEmission = FramebufferObject::Create(texEmissionColor, texGameWindowDepth);

		// �u���[���pFBO���쐬
		vec2Int bloom = currentFlame;
		for (auto& bfbo : fboBloom) {
			bloom /= 2;
			const auto& texBloom = TextureManager::CreateTexture(
				"FBO(color)", bloom.x, bloom.y, GL_RGBA16F);
			bfbo = FramebufferObject::Create(texBloom, nullptr);
		}

		// SAO�pFBO���쐬
		// �[�x�l�̏k���o�b�t�@
		const int maxMipLevel = static_cast<int>(fboSAODepth.size());
		const auto& texSAODepth = TextureManager::CreateTexture(
			"FBO(sao depth)", currentFlame.x / 2, currentFlame.y / 2, GL_R32F, maxMipLevel);
		for (int level = 0; level < maxMipLevel; ++level) {
			fboSAODepth[level] =
				FramebufferObject::Create(texSAODepth, nullptr, level, 0);
		}
		// �v�Z���ʃo�b�t�@
		const auto& texSAO = TextureManager::CreateTexture(
			"FBO(sao)", currentFlame.x / 2, currentFlame.y / 2, GL_R8);
		fboSAOCalc = FramebufferObject::Create(texSAO, nullptr);
		// �ڂ����o�b�t�@
		const auto& texSAOBlur = TextureManager::CreateTexture(
			"FBO(sao blur)", currentFlame.x / 2, currentFlame.y / 2, GL_R8);
		fboSAOBlur = FramebufferObject::Create(texSAOBlur, nullptr);

	}
}

/**
* �J���[�e�N�X�`����FBO�ɕ`�悷��
*
* @param fbo      �`���FBO
* @param texture  FBO�ɕ`�悷��e�N�X�`��
* @param wrapMode �e�N�X�`���ɐݒ肷�郉�b�v���[�h
*/
void Engine::DrawTextureToFbo(
	FramebufferObject& fbo, ProgramPipeline& prog, 
	TexturePtr texture, GLenum wrapMode)
{
	glUseProgram(prog);

	// VAO�ƃV�F�[�_��OpenGL�R���e�L�X�g�Ɋ��蓖�Ă�
	glBindVertexArray(*meshBuffer->GetVAO());

	// �`���FBO�����蓖�Ă�
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	const Texture& texFbo = *fbo.GetColorTexture();
	glViewport(0, 0, texFbo.GetWidth(), texFbo.GetHeight());

	// ���b�v���[�h��ύX
	texture->SetWrapMode(wrapMode);

	// �J���[�e�N�X�`�����g�p�����}�e���A�����쐬
	Material mt;
	mt.texBaseColor = texture;

	// �`���FBO�Ƀe�N�X�`����`��
	DrawSpritePlaneMesh(*windowMesh, prog, vec4(1), mt);

	glBindVertexArray(0);
}

/**
* �Q�[���G���W��������������
*
* @retval 0     ����ɏ��������ꂽ
* @retval 0�ȊO �G���[����������
*/
int Engine::Initialize()
{
	// GLFW�̏�����
	if (glfwInit() != GLFW_TRUE) {
		return 1; // ���������s
	}

	// �`��E�B���h�E�̍쐬
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	window = glfwCreateWindow(1280, 720, title.c_str(), nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return 1; // �E�B���h�E�쐬���s
	}

	// �A�C�R���摜��ǂݍ���ŃA�C�R���ɐݒ�
	const GLFWimage& iconImage = LoadIcon_as_GLFWimage("Res/Icon/Dragon.ico");
	if (iconImage.pixels) {
		glfwSetWindowIcon(window, 1, &iconImage);
	}

	// OpenGL�R���e�L�X�g�̍쐬
	glfwMakeContextCurrent(window);

	// OpenGL�֐��̃A�h���X���擾
	//GLFW��GLAD�ł͊֐��^���قȂ邽�߁ureinterpret_cast�v��GLAD���C�u�����̌^�ɃL���X�g
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		glfwTerminate();
		return 1; // �A�h���X�擾���s
	}

	// ���b�Z�[�W�R�[���o�b�N�̐ݒ�
	glDebugMessageCallback(DebugCallback, nullptr);

	// 2�̃V�F�[�_�������N
	{
		// �Q�[���E�B���h�E�V�F�[�_
		progWindow = 
			ProgramPipeline::Create("Simple2D.vert", "Simple2D.frag");

		// �ʏ�V�F�[�_
		progDefaultLit =
			ProgramPipeline::Create("DefaultLit.vert", "DefaultLit.frag");
		// �A�����b�g
		progUnlit =		
			ProgramPipeline::Create("Unlit.vert", "Unlit.frag");
		// �A�j���[�V����
		progAnimationLit =	
			ProgramPipeline::Create("AnimationLit.vert", "DefaultLit.frag");
		// �A�j���[�V�����A�����b�g
		progAnimationUnlit =
			ProgramPipeline::Create("AnimationLit.vert", "Unlit.frag");
		// �X�y�L����
		progSprcular = 
			ProgramPipeline::Create("DefaultLit.vert", "Sprcular.frag");
		// �����΂̉e�𔽉f�����邽�ʏ�V�F�[�_
		progDefaultLit_with_StoneShadow =
			ProgramPipeline::Create("DefaultLit.vert", "DefaultLit_with_StoneShadow.frag");	

		// ���̉e�����󂯂�V�F�[�_��z��ɓ����
		affectedLightPrograms = { 
			*progDefaultLit, *progAnimationLit, *progSprcular, *progDefaultLit_with_StoneShadow,
		};

		// �e�p�V�F�[�_
		progShadow =	// �V���h�E
			ProgramPipeline::Create("Shadow/Shadow.vert", "Shadow/Shadow.frag");	
		progShadowAnimation =	// �A�j���[�V�����V���h�E
			ProgramPipeline::Create("Shadow/ShadowAnimation.vert", "Shadow/Shadow.frag");	
		glProgramUniform1f(*progShadow, Loc::AlphaCutoff, 0.5f); // �J�b�g�I�t�l��ݒ肵�Ă���

		// �G�~�b�V�����p�V�F�[�_
		progEmission = 
			ProgramPipeline::Create("Unlit.vert", "Emission.frag");
		progEmissionAnimation = 
			ProgramPipeline::Create("AnimationUnlit.vert", "Emission.frag");

		// �u���[���p�V�F�[�_
		progHighPassFilter = 
			ProgramPipeline::Create("Simple2D.vert", "Bloom/HighPassFilter.frag");
		progDownSampling = 
			ProgramPipeline::Create("Simple2D.vert", "Bloom/DownSampling.frag");
		progUpSampling = 
			ProgramPipeline::Create("Simple2D.vert", "Bloom/UpSampling.frag");

		// �A���r�G���g�I�N���[�W�����p�V�F�[�_
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

		// �Q�[���E�B���h�E��|�X�g�G�t�F�N�g�V�F�[�_��
		// ���W�ϊ��s������j�t�H�[���ϐ��ɃR�s�[
		// (�X�v���C�g��FBO�̃e�N�X�`�����W�͋t�Ȃ̂�Y�����}�C�i�X�ɂ���)
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

	// FBO���쐬
	{
		// �Q�[���E�B���h�E�pFBO���쐬
		const vec2Int flame = GetFlamebufferSize();
		const auto& texGameWindowColor = TextureManager::CreateTexture(
			"FBO(color)", flame.x, flame.y, GL_RGBA16F);
		const auto& texGameWindowDepth = TextureManager::CreateTexture(
			"FBO(depth)", flame.x, flame.y, GL_DEPTH_COMPONENT32F);
		fboMainGameWindow = FramebufferObject::Create(texGameWindowColor, texGameWindowDepth);

		// �e�pFBO���쐬
		const auto& texShadow = TextureManager::CreateTexture(
			"FBO(shadow)", 2048, 2048, GL_DEPTH_COMPONENT32);
		fboShadow = FramebufferObject::Create(nullptr, texShadow);

		// �G�~�b�V�����pFBO���쐬
		const auto& texEmissionColor = TextureManager::CreateTexture(
			"FBO(emission)", flame.x, flame.y, GL_RGBA16F);
		/* �Q�[���E�B���h�E�Ɠ����[�x�e�N�X�`�����g�p */
		fboEmission = FramebufferObject::Create(texEmissionColor, texGameWindowDepth);

		// �u���[���pFBO���쐬
		vec2Int bloom = flame;
		for (auto& bfbo : fboBloom) {
			bloom /= 2;
			const auto& texBloom = TextureManager::CreateTexture(
				"FBO(bloom)", bloom.x, bloom.y, GL_RGBA16F);
			bfbo = FramebufferObject::Create(texBloom, nullptr);
		}

		// SAO�pFBO���쐬����
		// �[�x�l�̏k���o�b�t�@
		const int maxMipLevel = static_cast<int>(fboSAODepth.size());
		const auto& texSAODepth = TextureManager::CreateTexture(
			"FBO(sao depth)", flame.x / 2, flame.y / 2, GL_R32F, maxMipLevel);
		for (int level = 0; level < maxMipLevel; ++level) {
			fboSAODepth[level] = 
				FramebufferObject::Create(texSAODepth, nullptr, level, 0);
		}
		// �v�Z���ʃo�b�t�@
		const auto& texSAO = TextureManager::CreateTexture(
			"FBO(sao)", flame.x / 2, flame.y / 2, GL_R8);
		fboSAOCalc = FramebufferObject::Create(texSAO, nullptr);
		// �ڂ����o�b�t�@
		const auto& texSAOBlur = TextureManager::CreateTexture(
			"FBO(sao blur)", flame.x / 2, flame.y / 2, GL_R8);
		fboSAOBlur = FramebufferObject::Create(texSAOBlur, nullptr);
	}

	// �o�b�t�@���쐬
	meshBuffer = MeshBuffer::Create(10'000'000);
	lightBuffer = LightBuffer::Create();
	particleBuffer = ParticleBuffer::Create(1'000);

	// glTF�t�@�C���p�̃o�b�t�@���쐬
	const size_t maxAnimeModelCount = 8;   // �A�j���[�V�������郂�f����
	const size_t maxAnimeMatrixCount = 128; // 1���f���̃{�[����
	gltfFileBuffer = 
		GltfFileBuffer::Create(10'000'000, maxAnimeModelCount * maxAnimeMatrixCount);


	windowMesh = meshBuffer->CreatePlaneXY("PlaneXY");

	// OBJ�t�@�C����ǂݍ���
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


	// glTF�t�@�C����ǂݍ���
	gltfFileBuffer->LoadGltf("Res/Gltf/Dragon/Dragon.gltf");
	gltfFileBuffer->LoadGltf("Res/Gltf/Hero/Hero.gltf");


	// �C���v�b�g�̏�������
	InputManager::GetInstance().StartUp(window);
	// �C���v�b�g�V�X�e���փA�N�V�����o�^
	{
		// UI�A�N�V�����}�b�v���쐬
		{
			InputActionMap uiMap("UI");
			{
				// �^�C�~���O�����O
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_LEFT },
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_RIGHT	},
					{ InputKind::PadTrigger,	GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER	},
				};
				uiMap.RegisterAction("TimingRing", action);
			}

			// UI�A�N�V�����}�b�v��ǉ�
			InputSystem::GetInstance().actionMaps.push_back(uiMap);
		}

		// �v���C���[�A�N�V�����}�b�v���쐬
		{
			InputActionMap playerMap("Player");
			{
				// �W�����v
				std::vector<InputActionData> action = {
					{ InputKind::Key,		GLFW_KEY_SPACE },
					{ InputKind::PadButton, GLFW_GAMEPAD_BUTTON_A },
					{ InputKind::PadButton, GLFW_GAMEPAD_BUTTON_B },
				};
				playerMap.RegisterAction("Jump", action);
			} {
				// ����
				std::vector<InputActionData> action = {
					{ InputKind::Key,		GLFW_KEY_LEFT_SHIFT },
					{ InputKind::Key,		GLFW_KEY_RIGHT_SHIFT },
					{ InputKind::PadButton, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER },
				};
				playerMap.RegisterAction("Run", action);
			} {
				// ��U��
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_LEFT },
					{ InputKind::PadButton,		GLFW_GAMEPAD_BUTTON_X },
				};
				playerMap.RegisterAction("Attack", action);
			} {
				// ���U��
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_RIGHT	},
					{ InputKind::PadButton,		GLFW_GAMEPAD_BUTTON_Y },
				};
				playerMap.RegisterAction("StrongAttack", action);
			} {
				// �^�C�~���O�����O
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_LEFT },
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_RIGHT	},
					{ InputKind::PadTrigger,	GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER	},
				};
				playerMap.RegisterAction("TimingRing", action);
			} {
				// �^�[�Q�b�g�t�H�[�J�X
				std::vector<InputActionData> action = {
					{ InputKind::MouseButton,	GLFW_MOUSE_BUTTON_MIDDLE },
					{ InputKind::PadButton,		GLFW_GAMEPAD_BUTTON_RIGHT_THUMB	},
				};
				playerMap.RegisterAction("Targeting", action);
			} 
			
			{
				// �ړ�
				std::vector<InputActionData> action = {
					{ InputKind::MoveAction },
				};
				playerMap.RegisterAction("Move", action);
			} {
				// ���_
				std::vector<InputActionData> action = {
					{ InputKind::ViewAction },
				};
				playerMap.RegisterAction("View", action);
			}

			// �v���C���[�A�N�V�����}�b�v��ǉ�
			InputSystem::GetInstance().actionMaps.push_back(playerMap);
		}
	}

	// �Q�[���I�u�W�F�N�g�z��̗e�ʂ�\��
	gameObjects.reserve(500);
	gameObjects_tag.reserve(100);

	return 0; // ����ɏ��������ꂽ
}

/**
* �Q�[���G���W���̏�Ԃ��X�V����
*/
void Engine::Update()
{
	// �C���v�b�g�̏�Ԃ��X�V
	InputManager::GetInstance().UpdeteStates(deltaSeconds);

	// �t���[�J�����̍X�V
	if (freeCamera) {
		freeCamera->Update(deltaSeconds, *this);
	}

	// �Q�[�����̑O��t���[������̌o�ߎ���
	const float gameDeltaSeconds = 
		deltaSeconds * gameSpeed * timeScale;

	// �V�[���̐؂�ւ�
	if (nextScene) {
		if (scene) {
			scene->Finalize(*this);
		}

		nextScene->Initialize(*this);
		
		scene = std::move(nextScene);
	}

	// �V�[�����X�V
	if (scene) {
		scene->Update(*this, gameDeltaSeconds);
	}

	// ���ׂẴQ�[���I�u�W�F�N�g�̍X�V����
	UpdateGameObject(gameDeltaSeconds);

	// �폜�w�肳��Ă���Q�[���I�u�W�F�N�g���폜
	RemoveDestroyedGameObject();

	// ���C���J�����̐����m�F
	if (mainCamera) {
		if (mainCamera->IsDestroyed()) {
			// �폜����Ă��邽�߃��Z�b�g
			mainCamera.reset();
		}
	}

	// ���C���J�����̃J�����R���|�[�l���g�̐����m�F
	if (mainCamera) {
		if (!GetMainCamera_CameraComponent()) {
			// �J�����R���|�[�l���g���擾�ł��Ȃ�������
			// ���݂̃��C���J�����ɃJ�����R���|�[�l���g�����邩�T��
			size_t cameraCompIndex = 0;
			for (const auto& e : mainCamera->components) {
				if (std::dynamic_pointer_cast<Camera>(e)) {
					break;
				}
				++cameraCompIndex;
			}

			// �J�����R���|�[�l���g���Ȃ������烁�C���J���������Z�b�g����
			if (cameraCompIndex == mainCamera->components.size()) {
				mainCamera.reset();
			}
			else {
				// �J�����R���|�[�l���g���Ō���ֈړ�������
				/* ���̏����������Ă��鎞�_�ŃJ�����R���|�[�l���g���Ō���ł͂Ȃ� */
				std::swap(
					mainCamera->components[cameraCompIndex], mainCamera->components.back());
			}
		}
	}

	// ���s�����̐����m�F
	if (directionalLight) {
		if (directionalLight->IsDestroyed()) {
			// �폜����Ă��邽�߃��Z�b�g
			directionalLight.reset();
		}
	}


	if (!freeCamera) {
		if (const auto& cameraObj = GetMainCameraObject()) {
			// ���C�g��GPU�������ɃR�s�[����
			lightBuffer->UpdateShaderLight(cameraObj->GetWorldTransformMatrix());

			Camera* camera = GetMainCamera_CameraComponent();

			// �p�[�e�B�N�����X�V
			particleBuffer->Update(camera->GetViewMatrix(), gameDeltaSeconds);

			// 3D�I�[�f�B�I�p���X�i�[�ݒ�
			const vec3 pos = vec3(cameraObj->GetWorldTransformMatrix()[3]);
			const vec3 rignt = normalize(vec3(cameraObj->GetWorldTransformMatrix()[0]));
			EasyAudio::SetListenr(pos, rignt);
		}
	}
	else {
		// ���C�g��GPU�������ɃR�s�[����
		lightBuffer->UpdateShaderLight(freeCamera->transMat);

		// �p�[�e�B�N�����X�V
		particleBuffer->Update(freeCamera->GetViewMatrix(), gameDeltaSeconds);

		// 3D�I�[�f�B�I�p���X�i�[�ݒ�
		EasyAudio::SetListenr(freeCamera->position, vec3(freeCamera->transMat[0]));
	}
}

/**
* �Q�[���I�u�W�F�N�g�̏�Ԃ��X�V����
*
* @param deltaSeconds �O��̍X�V����̌o�ߎ���(�b)
*/
void Engine::UpdateGameObject(float deltaSeconds)
{
	// ���C���J�������Q�[���I�u�W�F�N�g�z��̂̐擪�Ɉړ�����
	/* ���C���J�������ŏ��ɍs���̂̓��C���J�����̏�����ɏ������s���Ă���ꍇ�����邩�� */
	if (const auto& cameraObj = GetMainCameraObject()) {
		if (gameObjects[0].get() != cameraObj) {
			// �z��̐擪�����C���J�����ł͂Ȃ����ߒT��
			const auto& mainCamera_itr =
				std::find_if(gameObjects.begin(), gameObjects.end(),
					[&cameraObj](const GameObjectPtr& p) { return p.get() == cameraObj; });

			// �擪�̃Q�[���I�u�W�F�N�g�Ɠ���ւ���
			std::swap(*mainCamera_itr, gameObjects[0]);
		}
	}

	// Update�֐������s
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

	// ���ׂẴQ�[���I�u�W�F�N�g�̍��W�ϊ��s����v�Z
	CalcGameObjectTransMat(calcStartIndex);

	// ���݂̂��ׂẴQ�[���I�u�W�F�N�g�̌v�Z�������������ߔz��T�C�Y��ۑ�����
	calcStartIndex = gameObjects.size();

	// EndUpdate�֐��̎��s
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

	// EndUpdate�Œǉ����ꂽ�Q�[���I�u�W�F�N�g�̍��W�ϊ��s����v�Z
	CalcGameObjectTransMat(calcStartIndex);
	calcStartIndex = gameObjects.size();


	// �Q�[���I�u�W�F�N�g�̏Փ˂�����
	std::vector<WorldColliderList> colliders;	// �R���C�_�[�̃R�s�[�z��
	colliders.reserve(gameObjects.size());

	// ���[���h���W�n�̏Փ˔�����쐬
	for (const auto& gObj : gameObjects) {
		if (!gObj->CheckActive()) {
			continue;
		}
		if (gObj->colliders.empty()) {
			continue;
		}

		// ���W�b�h�{�f�B������΁u�ڒn���Ă��Ȃ��v��Ԃɂ���
		if (const auto& rigid = gObj->GetRigidbody()) {
			rigid->isGrounded = false;
		}

		// �Q�[���I�u�W�F�N�g���ێ����Ă���R���C�_�[���̔z����쐬
		WorldColliderList wcList;
		wcList.reserve(gObj->colliders.size());
		for (const auto& col : gObj->colliders) {
			if (!col->isActive) {
				continue;
			}

			WorldCollider wc;
			// �I���W�i���̃R���C�_�[���R�s�[
			wc.origin = col;
			// �R���C�_�[�̍��W�����[���h���W�ɕϊ�
			wc.world = col->GetTransformedCollider(gObj->GetWorldTransformMatrix());

			wcList.push_back(wc);
		}

		//�R�s�[�R���C�_�[�z��Ɋi�[����
		colliders.push_back(wcList);
	}

	// �R���C�_�[�̏Փˏ���
	ColliderDetection(&colliders);

	// �R���C�_�[�̏Փˏ����Œǉ����ꂽ�Q�[���I�u�W�F�N�g�̍��W�ϊ��s����v�Z
	CalcGameObjectTransMat(calcStartIndex);
} // UpdateGameObject

/**
* �Q�[���I�u�W�F�N�g�̍��W�ϊ��s��v�Z�����s
* 
* @param calcStartIndex	�v�Z���J�n����C���f�b�N�X�ԍ�
*/
void Engine::CalcGameObjectTransMat(const size_t& calcStartIndex)
{
	// ���[�J�����W�ϊ��s����v�Z
	for (size_t i = calcStartIndex; i < gameObjects.size(); ++i) {
		GameObject* gObj = gameObjects[i].get();
		gObj->localTransMat =
			TransformMatrix(gObj->position, gObj->rotation, gObj->scale);
	}

	// ���[���h���W�ϊ��s����v�Z
	for (size_t i = calcStartIndex; i < gameObjects.size(); ++i) {
		GameObject* gObj = gameObjects[i].get();
		mat4 m = gObj->GetLocalTransformMatrix();

		// ���g�̂��ׂĂ̐e�I�u�W�F�N�g�̍��W�ϊ��s��Ɗ|�����킹��
		for (gObj = gObj->parent; gObj; gObj = gObj->parent) {
			m = gObj->GetLocalTransformMatrix() * m;
		}

		gameObjects[i]->worldTransMat = m;
	}
}

/**
* �j���\��̃Q�[���I�u�W�F�N�g���폜����
*/
void Engine::RemoveDestroyedGameObject()
{
	if (gameObjects.empty()) {
		return; // �Q�[���I�u�W�F�N�g�������Ă��Ȃ���Ή������Ȃ�
	}

	// �j���\��̗L���ŃQ�[���I�u�W�F�N�g�𕪂���
	const auto& destrObjBegin_itr =
		std::stable_partition(gameObjects.begin(), gameObjects.end(),
			[](const GameObjectPtr& e) { return !e->IsDestroyed(); });

	if (destrObjBegin_itr == gameObjects.end()) {
		return;	// �폜����Q�[���I�u�W�F�N�g���Ȃ�
	}

	// �j���\��̃I�u�W�F�N�g��ʂ̔z��Ɉړ�
	GameObjectList destrObjs(
		std::make_move_iterator(destrObjBegin_itr),
		std::make_move_iterator(gameObjects.end()));

	// �z�񂩂�ړ��ς݃I�u�W�F�N�g���폜
	gameObjects.erase(destrObjBegin_itr, gameObjects.end());

	// �j���\��̃I�u�W�F�N�g��OnDestroy�����s�ƍ폜�����I�u�W�F�N�g��Tag���擾
	std::vector<MyFName> destrObjTags;
	destrObjTags.reserve(destrObjs.size());
	for (const auto& e : destrObjs) {
		e->OnDestroy();

		// �I�u�W�F�N�g��Tag���擾���ۑ�
		for (const auto& tag : e->tags) {
			// ���łɓ���Tag���ۑ�����Ă��邩���ׂ�
			const auto& itr =
				std::find(destrObjTags.begin(), destrObjTags.end(), tag);
			if (itr != destrObjTags.end()) {
				continue;	// �����^�O���ǉ��ς�
			}

			destrObjTags.push_back(tag);
		}

		// ���C���J�������폜�����Ȃ烁�C���J�����ϐ������Z�b�g����
		if (e == mainCamera) {
			mainCamera.reset();
		}
	}

	// Tag�����Q�[���I�u�W�F�N�g�z�񂩂���폜����
	for (const MyFName& tag : destrObjTags) {
		// �����^�O�������Ă���I�u�W�F�N�g���擾
		const auto& objs = gameObjects_tag.find(tag);
		if (objs == gameObjects_tag.end()) {
			// �^�O�����݂��Ȃ���Δ�΂�
			continue;
		}

		// �폜�w�肳��Ă���v�f���W�߂�
		const auto& begin_itr =
			std::partition(objs->second.begin(), objs->second.end(),
				[](const GameObjectPtr& p) { return !p->IsDestroyed(); });
		if (begin_itr == objs->second.end()) {
			continue;	// �폜����^�O�I�u�W�F�N�g���Ȃ�
		}

		// �^�O�����I�u�W�F�N�g�z�񂩂���폜����
		objs->second.erase(begin_itr, objs->second.end());
	}

	// �����ō폜�����I�u�W�F�N�g(destrObjs)�����S�ɉ�������
}



/**
* �Q�[���G���W���̏�Ԃ�`�悷��
*/
void Engine::Render()
{
	// FBO���t���[���T�C�Y�ɍ��킹��
	MatchFboSize();

	// �Q�[���E�B���h�E�pFBO�ɕ`��
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);

	// �t���[���o�b�t�@�̑傫�����擾
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	// �r���[�|�[�g��ݒ�i�E�B���h�E�T�C�Y���ς���Ă��`��̔䗦���ێ�����j�@���`��E�B���h�E�T�C�Y�ł͂Ȃ��u�`�悷��͈́v�������Ă���
	glViewport(0, 0, fbWidth, fbHeight);

	// �o�b�N�o�b�t�@���폜����Ƃ��̐F���w��
	glClearColor(0, 0, 0, 1);
	//�o�b�N�o�b�t�@���폜
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// �`��Ɏg���V�F�[�_���w��
	glUseProgram(*progDefaultLit);

	// �A�j���[�V�����o�b�t�@���N���A
	gltfFileBuffer->ClearAnimationBuffer();

	// �`��̑O���������s
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

	// �A�j���[�V�����o�b�t�@��GPU�������ɃR�s�[����
	gltfFileBuffer->UploadAnimationBuffer();

	particleBuffer->UpdateSSBO();


	// �V�F�[�_�ɃJ�����p�����[�^��ݒ�
	{
		// �J�����p�����[�^��ݒ肷��V�F�[�_�z��
		const GLuint programs[] = {
			*progDefaultLit,*progUnlit,
			*progAnimationLit, *progAnimationUnlit,
			*progSprcular, *progDefaultLit_with_StoneShadow,
			*progEmission, *progEmissionAnimation,
			*particleBuffer->progParticle, *particleBuffer->progParticleEmission
		};

		if (!freeCamera) {
			/* ���O�Ƀ��C���J�����̗L�����`�F�b�N���Ă��邽��null�`�F�b�N�͂��Ȃ� */
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

	// �[�x�e�X�g��L����
	glEnable(GL_DEPTH_TEST);

	// ���ʃJ�����O��L�����i������`�悵�Ȃ��j
	glEnable(GL_CULL_FACE);

	// ������������L����
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// �����f�[�^��GPU�������ɃR�s�[
	{
		// ���s�����̃f�[�^���擾
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
			// ����
			glProgramUniform3fv(prog, Loc::AmbientLight, 1, &ambientLight.x);

			// ���s�����̃p�����[�^
			glProgramUniform3fv(prog, Loc::DirectionalLightColor, 1, &color.x);
			glProgramUniform3fv(prog, Loc::DirectionalLightDirection, 1, &direction.x);
		}
	}

	// �����΃f�[�^��GPU�������ɃR�s�[
	{
		const GameObjectList& stones = FindGameObjectsWithTag("FallStone");

		// �R�s�[����΂����邩
		if (stones.empty()) {
			// �Ȃ�
			glProgramUniform1i(*progDefaultLit_with_StoneShadow,
				Loc::FallStone::FallStoneCount, 0);
		}
		else {
			// ����
			std::vector<vec4> stoneDatas;
			stoneDatas.reserve(std::min(stones.size(), MaxShaderFallStoneCount));

			// �R�s�[���闎���΃p�����[�^���쐬
			for (const auto& s :stones) {
				if (!s->CheckActive() ||		// �A�N�e�B�u�ł͂Ȃ�
					!isValid(s) ||				// �폜����Ă���
					s->components.size() == 0)	// �R���|�[�l���g���Ȃ�
				{
					continue;
				}

				// �����΃R���|�[�l���g���擾
				const auto& fs = s->GetComponent<FallStone>();
				if (!fs) {
					continue;
				}

				// �p�����[�^�[��ǉ�
				stoneDatas.emplace_back(
					s->position.x, s->position.z,
					fs->param.maxStoneScale,
					1 - (fs->param.maxStoneScale - s->scale.x));

				// �ő�R�s�[���ɒB������I��
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

	// �Q�[���I�u�W�F�N�g�������_�[�L���[���ɕ��בւ���
	std::stable_sort(	/* �z����u���ёւ����[���v�𖞂����悤�v�f����ѕς��� */
		gameObjects.begin(), gameObjects.end(),
		[](const GameObjectPtr& a, const GameObjectPtr& b) {
			return a->renderQueue < b->renderQueue; });	/*renderQueue�������������O�ɗ���悤�ɂ���*/

	// �w�肳��ăL���[�̐擪�����������_��
	const auto FindRenderQueueBegin =
		[this](const GameObjectList::iterator findBegin, const int renderQueue)
		-> const GameObjectList::iterator
		{
			return std::lower_bound(
				findBegin, gameObjects.end(), renderQueue,
				[](const GameObjectPtr& e, const int value) { return e->renderQueue < value; });
		};	

	// stoneShadow�L���[�̐擪��gameObjects.begin()�Ɠ������ߌ������Ȃ�
	
	// geometry�L���[�̐擪������
	const auto& geometryBegin_itr =
		FindRenderQueueBegin(gameObjects.begin(), RenderQueue_geometry);

	// sprcular�L���[�̐擪������
	const auto& sprcularBegin_itr =
		FindRenderQueueBegin(geometryBegin_itr, RenderQueue_sprcular);

	// effect�L���[�̐擪������
	const auto& effectBegin_itr =
		FindRenderQueueBegin(sprcularBegin_itr, RenderQueue_effect);

	// transparent�L���[�̐擪������
	const auto& transparentBegin_itr =
		FindRenderQueueBegin(effectBegin_itr, RenderQueue_transparent);

	// overlay�L���[�̐擪������
	const auto& overlayBegin_itr =
		FindRenderQueueBegin(transparentBegin_itr, RenderQueue_overlay);

	// overlay�L���[(UI)�̗D��x���Ƀ\�[�g
	std::stable_sort(overlayBegin_itr, gameObjects.end(),
		[](const GameObjectPtr& a, const GameObjectPtr& b) {
			// ���ꂼ���UI���C�A�E�g���擾
			// ���C�A�E�g���Ȃ���������։�
			const auto& uiA = a->GetComponent<UILayout>();
			if (!uiA) return false;
			const auto& uiB = b->GetComponent<UILayout>();
			if (!uiB) return false;

			// �D��x���Ⴂ���̂�O�ֈړ�
			return uiA->priority < uiB->priority; });

	// �f�v�X�V���h�E�}�b�v���쐬
	CreateShadowMap(gameObjects.begin(), effectBegin_itr);

	// �`�����Q�[���E�B���h�E�t���[���o�b�t�@�ɖ߂�
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
	glViewport(0, 0, fbWidth, fbHeight);

	// ���C�g�pSSBO���o�C���h
	lightBuffer->Bind();

	// stoneShadow�L���[��`��i�s�����I�u�W�F�N�g�j
	DrawGameObject(*progDefaultLit_with_StoneShadow, *progUnlit, 
		0.5f, gameObjects.begin(), geometryBegin_itr);

	// geometry�L���[��`��i�s�����I�u�W�F�N�g�j
	DrawGameObject(*progDefaultLit, *progUnlit, 
		0.5f, geometryBegin_itr, sprcularBegin_itr);

	// sprcular�L���[��`��i���ʔ��˃I�u�W�F�N�g�j
	DrawGameObject(*progSprcular, *progUnlit,
		0.5f, sprcularBegin_itr, effectBegin_itr);

	// �Q�[���E�B���h�EFBO��AO��K��
	if (saoParam.isEnabled) {
		DrawAmbientOcclusion();
	}

	// �`���t���[���o�b�t�@��߂�
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
	glViewport(0, 0, fbWidth, fbHeight);

	// �X�J�C�X�t�B�A��`��
	DrawSkySphere(*progUnlit);

	// effect�L���[��`��
	glDepthMask(GL_FALSE);	// �[�x�o�b�t�@�ւ̏������݂��֎~
	glDisable(GL_CULL_FACE);// ���ʃJ�����O�𖳌���
	DrawGameObject(*progUnlit, *progUnlit, 
		0.0f, effectBegin_itr, transparentBegin_itr);
	glDepthMask(GL_TRUE);	// �[�x�o�b�t�@�ւ̏������݂�����
	glEnable(GL_CULL_FACE);	// ���ʃJ�����O��L����

	// �p�[�e�B�N����`��
	particleBuffer->Draw();

	// �Q�[���E�B���h�EFBO�Ƀu���[���G�t�F�N�g��K��
	if (screenBloomParam.isEnabled) {
		DrawBloomEffect(*fboMainGameWindow, screenBloomParam);
	}

	// �`���t���[���o�b�t�@��߂�
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
	glViewport(0, 0, fbWidth, fbHeight);

	// �[�x�e�X�g��L����
	glEnable(GL_DEPTH_TEST);	

	// ���ʃJ�����O�𖳌���
	glDisable(GL_CULL_FACE);

	// ������������L����
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// �R���C�_�[������
	if (debugWindow && showCollision) {
		ShowCollision(*progUnlit);
	}

	// transparent�L���[��`��i�������I�u�W�F�N�g�j
	glDepthMask(GL_FALSE); // �[�x�o�b�t�@�ւ̏������݂��֎~
	DrawGameObject(*progDefaultLit, *progUnlit, 
		0.0f, transparentBegin_itr, overlayBegin_itr);
	glDepthMask(GL_TRUE); // �[�x�o�b�t�@�ւ̏������݂�����

	// overlay�ȍ~�̃L���[��`��iUI�j
	glDisable(GL_DEPTH_TEST); // �[�x�e�X�g�𖳌����i���������邱�Ƃ�UI���ǂɖ��܂�Ȃ��Ȃ�j
	DrawGameObject(*progUnlit, *progUnlit, 
		0.0f, overlayBegin_itr, gameObjects.end());

	// ���C�g�pSSBO�̃o�C���h������
	lightBuffer->Unbind();


	// �G�~�b�V�����e�N�X�`��FBO���쐬
	if (emissionBloomParam.isEnabled)
	{	
		// �`�����G�~�b�V�����t���[���o�b�t�@�ɂ���
		glBindFramebuffer(GL_FRAMEBUFFER, *fboEmission);
		// �r���[�|�[�g���t���[���o�b�t�@�̃T�C�Y�ɍ��킹��
		const auto& texEmission = fboEmission->GetColorTexture();
		glViewport(0, 0, texEmission->GetWidth(), texEmission->GetHeight());
		
		// �o�b�N�o�b�t�@���폜����Ƃ��̐F���w��
		glClearColor(0, 0, 0, 1);
		
		// �o�b�t�@���N���A
		glClear(GL_COLOR_BUFFER_BIT);
		
		// �[�x�o�b�t�@�ւ̏������݂��֎~(�Q�[���E�B���h�E�Ɠ����[�x�e�N�X�`�����g�p���邽��)
		glDepthMask(GL_FALSE); 

		// �[�x�e�X�g��L����
		glEnable(GL_DEPTH_TEST);

		// ���ʃJ�����O��L�����i������`�悵�Ȃ��j
		glEnable(GL_CULL_FACE);

		// �t���O�����g�̐[�x�l���[�x�o�b�t�@�̒l�ȉ��̏ꍇ�ɕ`�������
		glDepthFunc(GL_LEQUAL);
		
		// stoneShadow�Egeometry�Esprcular�E�L���[��`��i�s�����I�u�W�F�N�g�j
		DrawEmissionGameObject(0.5f, gameObjects.begin(), effectBegin_itr);

		// effect�L���[��`��
		glDisable(GL_CULL_FACE);// ���ʃJ�����O�𖳌���
		DrawEmissionGameObject(0.0f, effectBegin_itr, transparentBegin_itr);
		glEnable(GL_CULL_FACE);	// ���ʃJ�����O��L����

		DrawSkySphere(*progEmission);

		// �p�[�e�B�N����`��
		particleBuffer->DrawEmission();

		// overlay�ȍ~�̃L���[��`��iUI�j
		glDisable(GL_DEPTH_TEST);	// �[�x�e�X�g�𖳌���
		glDisable(GL_CULL_FACE);	// ���ʃJ�����O�𖳌���
		DrawEmissionGameObject(0.0f, overlayBegin_itr, gameObjects.end());

		// �G�~�b�V����FBO�Ƀu���[���G�t�F�N�g��K��
		DrawBloomEffect(*fboEmission, emissionBloomParam);

		glDisable(GL_DEPTH_TEST);	// �[�x�e�X�g�𖳌���
		glDisable(GL_CULL_FACE);	// ���ʃJ�����O�𖳌���

		// �G�~�b�V����(�u���[��)FBO���Q�[���E�B���h�EFBO�ɉ��Z����
		DrawTextureToFbo(*fboMainGameWindow, *progWindow,
			fboEmission->GetColorTexture(), GL_CLAMP_TO_EDGE);

		// �[�x�e�X�g�ݒ�����̖߂�
		glDepthFunc(GL_LESS);
	}

	// �p�[�e�B�N���pSSBO�̃o�C���h������
	particleBuffer->Unbind();

#if 0
	// �}�b�v�̃`�F�b�N�p
	{
		// �`�����Q�[���E�B���h�E�t���[���o�b�t�@��
		glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
		glUseProgram(*progUnlit);

		// ������������L����
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

	// �f�t�H���g�t���[���o�b�t�@�Ɏw�肳��Ă���GBuffer��`�悷��
	{
		// �`�����f�t�H���g�t���[���o�b�t�@��
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, fbWidth, fbHeight);

		// �A���t�@�u�����h��L���ɖ߂�
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// �[�x�e�X�g�𖳌���
		glDisable(GL_DEPTH_TEST); 

		// VAO�ƃV�F�[�_��OpenGL�R���e�L�X�g�Ɋ��蓖�Ă�
		glUseProgram(*progWindow);
		glBindVertexArray(*meshBuffer->GetVAO());

		// �}�e���A�����쐬
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

		// �쐬�����}�e���A�����f�t�H���g�t���[���o�b�t�@�ɕ`��
		DrawSpritePlaneMesh(*windowMesh, *progWindow, vec4(1), mt);

		// VAO�V�F�[�_�̊��蓖�Ă�����
		glBindVertexArray(0);
	}

	//�t���[���o�b�t�@�̕\�����ƕ`�摤�����ւ���
	glfwSwapBuffers(window);
}

/**
* �f�v�X�V���h�E�}�b�v���쐬
* 
* @param begin			�`�悷��Q�[���I�u�W�F�N�g�͈͂̐擪
* @param end			�`�悷��Q�[���I�u�W�F�N�g�͈͂̏I�[
*/
void Engine::CreateShadowMap(
	GameObjectList::iterator begin, GameObjectList::iterator end)
{
	// �`���t���[���o�b�t�@��ύX
	glBindFramebuffer(GL_FRAMEBUFFER, *fboShadow);

	// �[�x�o�b�t�@���N���A
	glClear(GL_DEPTH_BUFFER_BIT);

	// �r���[�|�[�g���t���[���o�b�t�@�̃T�C�Y�ɍ��킹��
	const auto& texShadow = fboShadow->GetDepthTexture();
	glViewport(0, 0, texShadow->GetWidth(), texShadow->GetHeight());

	// VAO�ƃV�F�[�_��OpenGL�R���e�L�X�g�Ɋ��蓖�Ă�
	glBindVertexArray(*meshBuffer->GetVAO());
	glUseProgram(*progShadow);

	// �e�̕`��p�����[�^
	const float shadowAreaSize = 100; // �e�̕`��͈�(XY����)
	const float shadowNearZ = 1;      // �e�̕`��͈�(��Z����)
	const float shadowFarZ = 200;     // �e�̕`��͈�(��Z����)
	const float shadowCenterZ = (shadowNearZ + shadowFarZ) * 0.5f; // �`��͈͂̒��S
	const vec3 target = { 0, 0, 0 }; // �J�����̒����_
	vec3 eye;
	if (directionalLight) {	/* ���s�����̗L���͎��O�Ɋm�F�ς݂̂��߂����ł͍폜�`�F�b�N�͂��Ȃ� */
		eye = target - directionalLight->GetDirection() * shadowCenterZ; // �J�����̈ʒu
	}
	else {
		eye = target - vec3(0) * shadowCenterZ;
	}

	// �r���[�v���W�F�N�V�����s����v�Z
	const mat4 matShadowView = LookAt(eye, target, vec3(0, 1, 0));
	const mat4 matShadowProj = Orthogonal(
		-shadowAreaSize / 2, shadowAreaSize / 2,
		-shadowAreaSize / 2, shadowAreaSize / 2,
		shadowNearZ, shadowFarZ);
	const mat4 matShadow = matShadowProj * matShadowView;

	// �r���[�v���W�F�N�V�����s���GPU�������ɃR�s�[
	glProgramUniformMatrix4fv(*progShadow,
		Loc::ViewProjectionMatrix, 1, GL_FALSE, &matShadow[0].x);
	glProgramUniformMatrix4fv(*progShadowAnimation,
		Loc::ViewProjectionMatrix, 1, GL_FALSE, &matShadow[0].x);

	// ���b�V����`��
	for (GameObjectList::iterator i = begin; i != end; ++i) {
		const auto& gObj = *i;
		if (!gObj->CheckActive()) {
			// �A�N�e�B�u�ł͂Ȃ�
			continue;
		}

		for (const auto& r : gObj->renderers) {
			if (!r->isActive) {
				// �A�N�e�B�u�ł͂Ȃ�
				continue;
			}
			if (!r->castShadow) {
				// �e�𗎂Ƃ��Ȃ�
				continue;
			}

			//���_�A�g���r���[�g���o�C���h
			glBindVertexArray(*meshBuffer->GetVAO());

			switch (r->GetModelFormat())
			{
				// OBJ�EglTF(�A�j���[�V�����Ȃ�)��`��
			case ModelFormat::obj:
			case ModelFormat::gltfStatic:
				r->Draw(*progShadow);
				break;

				// glTF(�A�j���[�V��������)��`��
			case ModelFormat::gltfAnimated:
				glUseProgram(*progShadowAnimation);
				r->Draw(*progShadowAnimation);
				glUseProgram(*progShadow);
				break;
			}
		}
	} // for

	// �[�x�e�N�X�`�������蓖�Ă�
	glBindTextureUnit(Bind::Texture::Shadow, *texShadow);

	// �[�x�e�N�X�`�����W�ւ̕ϊ��s����쐬
	static const mat4 matTexture = {
		{ 0.5f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.5f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.5f, 0.0f },
		{ 0.5f, 0.5f, 0.5f, 1.0f }
	};

	// NDC���W�n����e�N�X�`�����W�n�֕ϊ�
	const mat4 matShadowTexture = matTexture * matShadow * matShadowView;

	// �@�������̕␳�l��ݒ�
	const float texelSize = shadowAreaSize / static_cast<float>(texShadow->GetWidth());

	for (const auto& prog : affectedLightPrograms) {
		// �V���h�E�e�N�X�`���s��
		glProgramUniformMatrix4fv(prog,
			Loc::ShadowTextureMatrix, 1, GL_FALSE, &matShadowTexture[0].x);

		// �@���␳�l
		glProgramUniform1f(prog, Loc::ShadowNormalOffset, texelSize);
	}
}

/**
* �Q�[���I�u�W�F�N�g�z���`�悷��
*
* @param progLit		���̉e�����󂯂�I�u�W�F�N�g�̕`��Ɏg���v���O����
* @param progUnlit		���̉e�����󂯂Ȃ��I�u�W�F�N�g�̕`��Ɏg���v���O����
* @param alphaCutoff	�t���O�����g��j�����鋫�E�l
* 
* @param begin			�`�悷��Q�[���I�u�W�F�N�g�͈͂̐擪
* @param end			�`�悷��Q�[���I�u�W�F�N�g�͈͂̏I�[
*/
void Engine::DrawGameObject(
	ProgramPipeline& progLit, ProgramPipeline& progUnlit, float alphaCutoff,
	GameObjectList::iterator begin, GameObjectList::iterator end)
{
	for (GameObjectList::iterator itr = begin; itr != end; ++itr) {
		const auto& gObj = *itr;
		if (!gObj->CheckActive()) {
			// �A�N�e�B�u�ł͂Ȃ�
			continue;
		}

		// �Q�[���I�u�W�F�N�g�̎����Ă���S�Ẵ��b�V����`��
		for (const auto& r : gObj->renderers) {
			if (!r->isActive) {
				continue;
			}

			// ���f���t�H�[�}�b�g�ƌ��̉e���̗L���ɂ���ăv���O������I��
			ProgramPipeline* prog = nullptr;
			switch (r->GetModelFormat())
			{
				// OBJ�EglTF(�A�j���[�V�����Ȃ�)��`��
			case ModelFormat::obj:
			case ModelFormat::gltfStatic:
				prog = &(r->useLighting ? progLit : progUnlit);
				break;

				// glTF(�A�j���[�V��������)��`��
			case ModelFormat::gltfAnimated:
				prog = &(r->useLighting ? *progAnimationLit : *progAnimationUnlit);
				break;

			default:
				continue;
			}

			glUseProgram(*prog);

			//���_�A�g���r���[�g���o�C���h
			glBindVertexArray(*meshBuffer->GetVAO());

			// �t���O�����g��j�����鋫�E�l��ݒ�
			glProgramUniform1f(*prog, Loc::AlphaCutoff, alphaCutoff);

			r->Draw(*prog);
		}
	}
}

/**
* �X�J�C�X�t�B�A��`�悷��
* 
* @param prog	�`��Ɏg���v���O����
*/
void Engine::DrawSkySphere(ProgramPipeline& prog)
{
	// �V�[���ɃX�J�C�X�t�B�A���ݒ肳��Ă��Ȃ��ꍇ�͕`�悵�Ȃ�
	if (!skySphere || !scene->skysphereMaterial) {
		return;
	}

	glDepthMask(GL_FALSE); // �[�x�o�b�t�@�ւ̏������݂��֎~

	// �A�����b�g�V�F�[�_�ŕ`��
	glUseProgram(prog);
	glBindVertexArray(*meshBuffer->GetVAO());

	// �X�J�C�X�t�B�A���f���̔��a��0.5m�Ɖ��肵�A�`��͈͂�95%�̑傫���Ɋg��
	static const float far = 1000; // �`��͈͂̍ő�l
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

	bool hasUniformColor = 	// �J���[���j�t�H�[���̗L��
		glGetUniformLocation(prog, "color") >= 0;
	if (hasUniformColor) {
		// �F�̓}�e���A���J���[�Œ�������̂ŃI�u�W�F�N�g�J���[�͔��ɐݒ�
		static const vec4 color(1);
		glProgramUniform4fv(prog, Loc::Color, 1, &color.x);
	}

	// �X�J�C�X�t�B�A��`��
	const MaterialList materials(1, scene->skysphereMaterial);
	DrawStaticMesh(*skySphere, prog, vec4(1), materials);
	
	glProgramUniform1f(prog, Loc::AlphaCutoff, 0);
	glDepthMask(GL_TRUE); // �[�x�o�b�t�@�ւ̏������݂�����
}

/**
* �Q�[���I�u�W�F�N�g�z���`�悷��
*
* @param alphaCutoff	�t���O�����g��j�����鋫�E�l
*
* @param begin			�`�悷��Q�[���I�u�W�F�N�g�͈͂̐擪
* @param end			�`�悷��Q�[���I�u�W�F�N�g�͈͂̏I�[
*/
void Engine::DrawEmissionGameObject(
	float alphaCutoff,
	GameObjectList::iterator begin, GameObjectList::iterator end)
{
	// �t���O�����g��j�����鋫�E�l��ݒ�
	glProgramUniform1f(*progEmission, 
		Loc::AlphaCutoff, alphaCutoff);
	glProgramUniform1f(*progEmissionAnimation,
		Loc::AlphaCutoff, alphaCutoff);

	glUseProgram(*progEmission);

	for (GameObjectList::iterator itr = begin; itr != end; ++itr) {
		const auto& gObj = *itr;
		if (!gObj->CheckActive()) {
			// �A�N�e�B�u�ł͂Ȃ�
			continue;
		}

		for (const auto& r : gObj->renderers) {
			if (!r->isActive) {
				// �A�N�e�B�u�ł͂Ȃ�
				continue;
			}

			//���_�A�g���r���[�g���o�C���h
			glBindVertexArray(*meshBuffer->GetVAO());

			switch (r->GetModelFormat())
			{
				// OBJ�EglTF(�A�j���[�V�����Ȃ�)��`��
			case ModelFormat::obj:
			case ModelFormat::gltfStatic:
				r->Draw(*progEmission);
				break;

				// glTF(�A�j���[�V��������)��`��
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
* �u���[����`��
* 
* @param fbo		�u���[����K������FBO
* @param param		�u���[���p�����[�^
*/
void Engine::DrawBloomEffect(
	FramebufferObject& fbo, const BloomParam& param)
{
	// �u���[���G�t�F�N�g�p��GL�R���e�L�X�g��ݒ�
	glDisable(GL_DEPTH_TEST);	// �[�x�e�X�g�𖳌���
	glDisable(GL_CULL_FACE);	// ���ʃJ�����O�𖳌���

	// 1. �Q�[����ʗpFBO���獂�P�x�����𒊏o
	glDisable(GL_BLEND); // �����Ȃ�(�������������s��Ȃ�����)
	{
		// ���P�x������FBO�̖����������Z�����̂ŁA���̉e����ł��������߂ɖ����Ŋ���
			/*���Z���v�̌��ʂ�1�{�ɂȂ�悤�ɁA�u���[���̋�����FBO�̖����ŏ��Z*/
		const float s = param.strength / static_cast<float>(std::size(fboBloom));
		glProgramUniform2f(
			*progHighPassFilter, Loc::Bloom::BloomInfo, param.threshold, s);

		DrawTextureToFbo(*fboBloom[0], *progHighPassFilter,
			fbo.GetColorTexture(), GL_CLAMP_TO_BORDER);
	}

	// 2. �k���ڂ������s���A1�i������FBO�ɃR�s�[
	for (size_t i = 1; i < fboBloom.size(); ++i) { /*fboBloom[0] �` fboBloom[5]*/
		DrawTextureToFbo(*fboBloom[i], *progDownSampling,
			fboBloom[i - 1]->GetColorTexture(), GL_CLAMP_TO_BORDER);
	}

	// 3. �g��ڂ������s���A1�i�傫��FBO�ɉ��Z����
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE); // ���Z�����i���ʂ̐F = �V�F�[�_�o�� * 1.0(GL_ONE) + FBO * 1.0(GL_ONE) = �V�F�[�_�o�� + FBO�j
	for (size_t i = fboBloom.size() - 1; i > 0; ) {
		--i;
		DrawTextureToFbo(*fboBloom[i], *progUpSampling,
			fboBloom[i + 1]->GetColorTexture(), GL_CLAMP_TO_EDGE);
	}

	// 4. �Ō�̊g��ڂ������s���A�Q�[����ʗpFBO�ɉ��Z����
	{
		DrawTextureToFbo(fbo, *progWindow,
			fboBloom[0]->GetColorTexture(), GL_CLAMP_TO_EDGE);	/*�Q�[����ʗpFBO*/
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

/**
* �A���r�G���g�I�N���[�W������`��
*/
void Engine::DrawAmbientOcclusion()
{
	// �J�����O�A�[�x�e�X�g�A�A���t�@�u�����h�𖳌���
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	// �J�����p�����[�^���擾
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

	// 1. �[�x�l����`�����ɕϊ�
	{
		// �J�����̋߁E�����ʒl�����j�t�H�[���ϐ��ɃR�s�[
		glProgramUniform2f(*progSAORestoreDepth, Loc::Sao::CameraNearFar,
			near, far);

		DrawTextureToFbo(*fboSAODepth[0], *progSAORestoreDepth,
			fboMainGameWindow->GetDepthTexture(), GL_CLAMP_TO_BORDER);
	}

	// 2. �k���o�b�t�@���쐬
	{
		for (int i = 1; i < fboSAODepth.size(); ++i) {
			glProgramUniform1i(*progSAODownSampling, Loc::Sao::MipLevel, i - 1);
			DrawTextureToFbo(*fboSAODepth[i], *progSAODownSampling,
				fboSAODepth[0]->GetColorTexture(), GL_CLAMP_TO_BORDER);
		}
	}

	// 3. SAO���v�Z
	{
		// SAO����p�p�����[�^��ݒ�
		glProgramUniform4f(*progSAOCalc, Loc::Sao::SaoInfo,
			saoParam.radius * saoParam.radius,
			0.5f * saoParam.radius * fovScale.y,
			saoParam.bias,
			saoParam.intensity / pow(saoParam.radius, 6.0f)
		);

		// NDC���W�����_���W�ɕϊ�����p�����[�^��ݒ�
		const float w = static_cast<float>(fboSAODepth[0]->GetWidth());
		const float h = static_cast<float>(fboSAODepth[0]->GetHeight());
		const float aspectRatio = w / h;
		const float invFovScale = 1.0f / fovScale.y;
		glProgramUniform2f(*progSAOCalc, Loc::Sao::NdcToView,
			invFovScale * aspectRatio, invFovScale);
	
		DrawTextureToFbo(*fboSAOCalc, *progSAOCalc,
			fboSAODepth[0]->GetColorTexture(), GL_CLAMP_TO_BORDER);
	}

	// 4. SAO�̌��ʂ��ڂ���
	{
		glUseProgram(*progSAOBlur);
		glBindFramebuffer(GL_FRAMEBUFFER, *fboSAOBlur);

		// �Օ����e�N�X�`��
		GLuint texColor = *fboSAOCalc->GetColorTexture();
		glBindTextures(0, 1, &texColor);
		
		// ���`�[�x�l
		GLuint texDepth = *fboSAODepth[0]->GetColorTexture();
		glBindTextures(1, 1, &texDepth);

		glBindVertexArray(*meshBuffer->GetVAO());
		for (const auto& prim : windowMesh->primitives) {
			DrawPrimitive(prim);
		}
	}

	// 5. �A���r�G���g�I�N���[�W�������Q�[���E�B���h�E�ɍ���
	{
		glUseProgram(*progSAO);
		glBindFramebuffer(GL_FRAMEBUFFER, *fboMainGameWindow);
		glViewport(0, 0, fboMainGameWindow->GetWidth(), fboMainGameWindow->GetHeight());

		// SAO�e�N�X�`��
		GLuint texShield = *fboSAOBlur->GetColorTexture();
		glBindTextures(0, 1, &texShield);
		
		// �Q�[���E�B���h�E�e�N�X�`��
		GLuint texColor = *fboMainGameWindow->GetColorTexture();
		glBindTextures(1, 1, &texColor);

		glBindVertexArray(*meshBuffer->GetVAO());
		for (const auto& prim : windowMesh->primitives) {
			DrawPrimitive(prim);
		}
	}

	// �J�����O�A�[�x�e�X�g�A�A���t�@�u�����h��L����
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}



/**
* �R���W������\������
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
* ImGui�ɂ����\��
*/
void Engine::RenderImGui()
{
	glfwMakeContextCurrent(debugWindow);

	// �o�b�N�o�b�t�@���폜
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// �V�X�e��
	{
		ImGui::Begin("System", NULL, ImGuiWindowFlags_NoMove);
		ImGui::Text("FPS: %f", fps);	// FPS�\��
		ImGui::Text("DeltaTime: %f", deltaSeconds);	// �O��X�V����̌o�ߎ��ԕ\��

		{
			ImGui::SeparatorText("");

			// �t���[�J�����̎g�p�L��
			bool useFreeCamera = (freeCamera != nullptr);
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImGui::Checkbox("FreeCamera", &useFreeCamera);
			ImGui::PopStyleColor();
			if (useFreeCamera) {
				// �t���[�J�������܂��쐬����Ă��Ȃ�������쐬
				if (!freeCamera) {
					// ���݂̃��C���J�����̈ʒu���擾
					const auto& cameraObj = GetMainCameraObject();
					vec3 pos, rot, sca;
					Decompose(cameraObj->GetWorldTransformMatrix(), pos, rot, sca);		
					
					freeCamera = new FreeRoamCamera(pos, rot);

					// �C���v�b�g�V�X�e���𖳌���
					InputSystem::GetInstance().isEnabled = false;
				}
			}
			else {
				// �t���[�J�������c���Ă�����폜
				if (freeCamera) {
					delete freeCamera;
					freeCamera = nullptr;

					// �C���v�b�g�V�X�e����L����
					InputSystem::GetInstance().isEnabled = true;
				}
			}

			// �Q�[���X�s�[�h
			ImGui::DragFloat("GameSpeed", &gameSpeed);
			gameSpeed = std::max(gameSpeed, 0.0f);
			if (ImGui::Button("Reset")) gameSpeed = 1.0f;
			ImGui::SameLine();
			if (ImGui::Button("Stop"))	gameSpeed = 0.0f;
			ImGui::Spacing();

			// �^�C���X�P�[��
			ImGui::DragFloat("TimeScale", &timeScale);
			timeScale = std::max(timeScale, 0.0f);
			ImGui::Spacing();

			// �R���W�����\���L���E�\���F�ݒ�
			ImGui::Checkbox("ShowCollision", &showCollision);
			if (ImGui::TreeNode("CollisionColor")) {
				ImGui::ColorEdit4("General",	&collisionColor[0].x);
				ImGui::ColorEdit4("Deactive",	&collisionColor[1].x);
				ImGui::ColorEdit4("Trigger",	&collisionColor[2].x);
				ImGui::ColorEdit4("Static",		&collisionColor[3].x);

				ImGui::TreePop();
			}
			ImGui::Spacing();

			// ����
			ImGui::Text("AmbientLight");
			ImGui::ColorEdit3("Color", &ambientLight.x);
			ImGui::Spacing();

			// �V�[��
			if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
				// ���݃V�[���̏��
				if (ImGui::TreeNode("CurrentScene Info")) {
					scene->DrawImGui();
					ImGui::TreePop();
				}
				ImGui::Spacing();

				// �V�[���Ҍ��{�^���쐬�}�N��
				#define CREATE_SCENE_CHANGE_BUTTON(sceneName, sceneClass) \
					do { \
						if (ImGui::Button(sceneName)) { \
							SetNextScene<sceneClass>(); \
							EasyAudio::Stop(AudioPlayer::bgm); \
						} \
				    } while (0)    

				// �e�V�[���̊Ҍ��{�^���쐬
				CREATE_SCENE_CHANGE_BUTTON("Debug", DebugSnene);
				CREATE_SCENE_CHANGE_BUTTON("Title", TitleScene);
				CREATE_SCENE_CHANGE_BUTTON("LoadMainGame", LoadMainGameScene);
				CREATE_SCENE_CHANGE_BUTTON("MainGame", MainGameScene);
				CREATE_SCENE_CHANGE_BUTTON("BackToTitle", BackToTitleScene);

				ImGui::TreePop();
			}

			// GBuffer�̕\���؂�ւ�
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
			// �Q�[���I�u�W�F�N�g���\��
			ImGui::SeparatorText("Information");
			ImGui::Text("GameObjects:     %d", static_cast<int>(gameObjects.size()));
			
			// �^�O�����Q�[���I�u�W�F�N�g���\��
			int tagCnt = 0;
			for (const auto& pair : gameObjects_tag) {
				tagCnt += static_cast<int>(pair.second.size());
			}
			ImGui::Text("GameObjectTags:  %d", tagCnt);
			// �e�^�O���Ƃɐݒ肳��Ă���Q�[���I�u�W�F�N�g���\��
			if (ImGui::TreeNode("Tag Info")) {
				for (const auto& pair : gameObjects_tag) {
					std::string name = pair.first.GetName();
					if (name.size() < 16) {
						name.resize(16, ' '); // ����Ȃ��������󔒂Ŗ��߂�
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

	// �Q�[���I�u�W�F�N�g
	{
		ImGui::Begin("GameObject", NULL, ImGuiWindowFlags_NoMove);

		// �Q�[���I�u�W�F�N�g�̐e�q�֌W���l�����ĕ\������֐�
		std::function<void(GameObject*)> GameObject_DrawImGui =
			[this, &GameObject_DrawImGui](GameObject* obj)
			{
				ImGuiTreeNodeFlags node_flags =
					ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_OpenOnDoubleClick |
					ImGuiTreeNodeFlags_SpanAvailWidth;

				// �I�����Ă���Q�[���I�u�W�F�N�g��
				if (obj == selectingGameObjct.lock().get()) {
					node_flags |= ImGuiTreeNodeFlags_Selected;
				}

				// �q�I�u�W�F�N�g�������Ă��邩
				if (obj->GetChildCount() == 0) {
					node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					ImGui::TreeNodeEx(
						static_cast<void*>(obj), node_flags, obj->name.GetName().c_str());

					// �I������Ă��邩
					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
						// �Q�[���I�u�W�F�N�g�|�C���^��z�񂩂�T��,
						// �I���I�u�W�F�N�g�ɐݒ肷��
						auto itr = std::find_if(gameObjects.begin(), gameObjects.end(),
							[&obj](const GameObjectPtr& p) { return p.get() == obj; });

						/*�K�������邽��null�`���b�N�͂��Ȃ�*/
						selectingGameObjct = *itr;
					}
				}
				else {
					bool openTreeNode =
						ImGui::TreeNodeEx(
							static_cast<void*>(obj), node_flags, obj->name.GetName().c_str());

					// �I������Ă��邩
					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
						// �Q�[���I�u�W�F�N�g�|�C���^��z�񂩂�T��,
						// �I���I�u�W�F�N�g�ɐݒ肷��
						const auto& itr = std::find_if(gameObjects.begin(), gameObjects.end(),
							[&obj](const GameObjectPtr& p) { return p.get() == obj; });

						/*�K�������邽��null�`���b�N�͂��Ȃ�*/
						selectingGameObjct = *itr;
					}

					if (openTreeNode) {
						// �m�[�h���J���Ă�����q�����\������
						for (const auto& c : obj->children) {
							GameObject_DrawImGui(c);
						}
						ImGui::TreePop();
					}
				}
			};

		//�S�ẴQ�[���I�u�W�F�N�g�̃p�����[�^�[��\��
		for (const auto& obj : gameObjects) {
			if (obj->GetParent()) {
				// �e���ݒ肳��Ă������΂�
				continue;
			}

			// �Q�[���I�u�W�F�N�g��\������
			GameObject_DrawImGui(obj.get());
		}
		ImGui::End();
	}

	// �C���X�y�N�^�[
	{
		ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_NoMove);
		if (const auto& sobj = selectingGameObjct.lock()) {
			sobj->DrawImGui();
		}
		ImGui::End();
	}

	// ImGuiDemo��\��
	{
		//ImGui::ShowDemoWindow();
	}

	// ���O��\��
	{
		LogBuffer::GetInstance().DrawImGui_AllLogs();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(debugWindow);
}
