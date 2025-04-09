#include "DebugWindow.h"

#if 0
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "LoadIcon.h"


DebugWindow::DebugWindow()
{
	// �f�o�b�N�E�B���h�E���쐬����
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(1000, 750, title.c_str(), nullptr, nullptr);

	if (window) {
		// �A�C�R���摜��ǂݍ���ŃA�C�R���ɐݒ�
		const GLFWimage& iconImage =
			LoadIcon_as_GLFWImage("Res/Icon/DebugWindow.ico");

		if (iconImage.pixels) {
			glfwSetWindowIcon(window, 1, &iconImage);
		}

		// ImGui�̏�����
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		const char* glsl_version = "#version 450";
		ImGui_ImplOpenGL3_Init(glsl_version);
	}
}

DebugWindow::~DebugWindow()
{
	// �f�o�b�N�E�B���h�E���I������
	glfwDestroyWindow(window);

	// ImGui�̏I��
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
#endif
