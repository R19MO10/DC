/**
* @file LogBuffer.cpp
*/
#include "LogBuffer.h"

#include <GLFW/glfw3.h>
#include <imgui.h>


void LogBuffer::AddLog(const std::string& message)
{
	// ���O�̐V�K�ǉ�����~���Ȃ珈�����Ȃ�
	if (stopAddLog) return;

	// ���O�ǉ����Ԃ��擾
	const float time = static_cast<float>(glfwGetTime());

	logs.emplace_back(time, message);
}


void LogBuffer::DrawImGui_AllLogs()
{
	ImGui::Begin("Debug Log", NULL, ImGuiWindowFlags_NoMove);

	// ���O�̐V�K�쐬���~�E�Đ�����
	if (ImGui::Button(stopAddLog ? "Play" : "Stop")) {
		stopAddLog = !stopAddLog;
	}
	ImGui::SameLine();

	// ���O�̋L�^������������
	if (ImGui::Button("Clear")) {
		logs.clear();
	}

	// ImGui�E�B���h�E�ɘg��\������
	ImGui::BeginChild("##debuglog", ImVec2(0.0f, 0.0f), true);	
		
	// ���O���o��
	for (const auto& log : logs) {
		ImGui::Text("[%.3f] ", log.first);	// �L�^���Ԃ�\���i������3�܂Łj
		ImGui::SameLine();
		ImGui::Text(log.second.c_str());	// ���O���e�̕\��
	}

	// �ŐV�̃��O����Ɍ�����悤�ɃI�[�g�X�N���[������
	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::EndChild();
	ImGui::End();
}
