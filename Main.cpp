/**
* @file Main.cpp
*/
#include "glad/glad.h"	/*���ŏ��ɃC���N���[�h����K�v�����遦*/
#include "Engine/Engine.h"
#include "../../Scene/TitleScene.h"
#include "../../Scene/MainGameScene.h"
#include "../../Scene/DebugSnene.h"
#include <GLFW/glfw3.h>

#pragma warning(push)
#pragma warning(disable:4005)
#include <Windows.h>
#pragma warning(pop)

/**
* �G���g���[�|�C���g
*/
int WINAPI WinMain(
	_In_ HINSTANCE hInstnce,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{ 
	Engine engine;
	engine.SetNextScene<TitleScene>();
	return engine.Run();
}
