#pragma once

#if 0
#include <string>
#include <memory>
#include "../Math/MyMath.h"

struct GLFWwindow;
class GameObject;
using GameObject_WeakPtr = std::weak_ptr<GameObject>;

class DebugWindow
{
private:
	GLFWwindow* window = nullptr;				// �E�B���h�E�I�u�W�F�N�g
	const std::string title = "Debug ImGui";	// �E�B���h�E�^�C�g��

	GameObject_WeakPtr selectingGameObjct;	// �I�𒆂̃Q�[���I�u�W�F�N�g

	bool showCollision = false;	// �R���W����������
	vec4 collisionColor[4] = {	// �R���W�����\���p�J���[
		{ 1.0f, 0.0f, 0.2f, 0.5f },	// �ʏ�
		{ 0.1f, 0.1f, 0.1f, 0.5f },	// ��A�N�e�B�u
		{ 1.0f, 1.0f, 0.0f, 0.5f },	// �g���K�[
		{ 0.0f, 1.0f, 0.5f, 0.5f },	// �X�^�e�B�b�N
	};

public:
	DebugWindow();
	virtual ~DebugWindow();

public:
	RenderImGui()
};

#endif