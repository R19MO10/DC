/**
* @file Mouse.cpp
*/
#include "Mouse.h"

#include "../../Engine/DebugLog.h"
#include <GLFW/glfw3.h>

void Mouse::StartUp(GLFWwindow* window)
{
	this->window = window;

	// �}�E�X���W�̌v�Z
	double x, y;
	glfwGetCursorPos(window, &x, &y);	// �X�N���[�����W�n�̃J�[�\�����W
	const vec2 pos = { static_cast<float>(x), static_cast<float>(y) };

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	const vec2 framebufferSize = { static_cast<float>(w), static_cast<float>(h) };

	const float aspectRatio = framebufferSize.x / framebufferSize.y;

	prevMousePos = mousePos = {
		(pos.x / framebufferSize.x * 2 - 1) * aspectRatio,
		(pos.y / framebufferSize.y * 2 - 1) * -1 };
}

void Mouse::UpdeteStates(float deltaSeconds)
{
	if (!window) {
		LOG_ERROR("[Mouse]�Fwindow���ݒ肳��Ă��܂���BStartUp�����s����Ă��Ȃ��\��������܂��B");
		return;
	}

	// �O�t���[���̃{�^����Ԃ�ۑ�
	prevButtonStates.swap(buttonStates);
	buttonStates.clear();

	// �O�t���[���̃}�E�X���W��ۑ�
	prevMousePos = mousePos;

	// �}�E�X�̃{�^����Ԃ��擾
	for (int bCode = GLFW_MOUSE_BUTTON_LEFT; bCode <= GLFW_MOUSE_BUTTON_MIDDLE; ++bCode) {
		if (glfwGetMouseButton(window, bCode) == GLFW_PRESS) {
			buttonStates.insert(bCode);
		}
	}

	// �}�E�X���W�̌v�Z
	double x, y;
	glfwGetCursorPos(window, &x, &y);	// �X�N���[�����W�n�̃J�[�\�����W
	const vec2 pos = { static_cast<float>(x), static_cast<float>(y) };

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	const vec2 framebufferSize = { static_cast<float>(w), static_cast<float>(h) };

	const float aspectRatio = framebufferSize.x / framebufferSize.y;

	/*
	UILayer�̍��W�n�ɍ��킹�邽�߂ɁA�X�N���[�����W�n����J�������W�n(Z=-1)�ɕϊ�����
		�J�������W�n�̃}�E�X���W.x =
			((�X�N���[�����W�n�̃}�E�X���W.x / ��ʃT�C�Y.x) * 2 - 1) * �A�X�y�N�g��

		�J�������W�n�̃}�E�X���W.y =
			((�X�N���[�����W�n�̃}�E�X���W.y / ��ʃT�C�Y.y) * 2 - 1) * -1

		�uY���W�̎���-1���|���Ă���̂́A�X�N���[�����W�n�ƃJ�������W�n�ł�Y���̕������t������v
	*/
	mousePos = {
		(pos.x / framebufferSize.x * 2 - 1) * aspectRatio,
		(pos.y / framebufferSize.y * 2 - 1) * -1 };
}