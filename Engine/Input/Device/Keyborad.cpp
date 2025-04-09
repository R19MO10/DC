/**
* @file Keyboard.cpp
*/
#include "Keyborad.h"

#include "../../Engine/DebugLog.h"
#include <GLFW/glfw3.h>

void Keyboard::StartUp(GLFWwindow* window) 
{
	this->window = window;
}

void Keyboard::UpdeteStates(float deltaSeconds)
{
	if (!window) {
		LOG_ERROR("window���ݒ肳��Ă��܂���BStartUp�����s����Ă��Ȃ��\��������܂��B");
		return;
	}

	// �O�t���[���ɉ����ꂽ�{�^�����i�[
	prevKeyStates.swap(keyStates);
	keyStates.clear();

	// PrintableKey�̏�Ԃ��擾
	for (int keyCode = GLFW_KEY_SPACE; keyCode <= GLFW_KEY_GRAVE_ACCENT; ++keyCode) {
		if (glfwGetKey(window, keyCode) == GLFW_PRESS) {
			keyStates.insert(keyCode);
		}
	}

	// FunctionKey�̏�Ԃ��擾
	for (int keyCode = GLFW_KEY_ESCAPE; keyCode <= GLFW_KEY_MENU; ++keyCode) {
		if (glfwGetKey(window, keyCode) == GLFW_PRESS) {
			keyStates.insert(keyCode);
		}
	}

}