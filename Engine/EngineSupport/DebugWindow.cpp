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
	// デバックウィンドウを作成する
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(1000, 750, title.c_str(), nullptr, nullptr);

	if (window) {
		// アイコン画像を読み込んでアイコンに設定
		const GLFWimage& iconImage =
			LoadIcon_as_GLFWImage("Res/Icon/DebugWindow.ico");

		if (iconImage.pixels) {
			glfwSetWindowIcon(window, 1, &iconImage);
		}

		// ImGuiの初期化
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		const char* glsl_version = "#version 450";
		ImGui_ImplOpenGL3_Init(glsl_version);
	}
}

DebugWindow::~DebugWindow()
{
	// デバックウィンドウを終了する
	glfwDestroyWindow(window);

	// ImGuiの終了
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
#endif
