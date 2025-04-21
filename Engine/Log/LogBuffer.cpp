/**
* @file LogBuffer.cpp
*/
#include "LogBuffer.h"

#include <GLFW/glfw3.h>
#include <imgui.h>


void LogBuffer::AddLog(const std::string& message)
{
	// ログの新規追加が停止中なら処理しない
	if (stopAddLog) return;

	// ログ追加時間を取得
	const float time = static_cast<float>(glfwGetTime());

	logs.emplace_back(time, message);
}


void LogBuffer::DrawImGui_AllLogs()
{
	ImGui::Begin("Debug Log", NULL, ImGuiWindowFlags_NoMove);

	// ログの新規作成を停止・再生する
	if (ImGui::Button(stopAddLog ? "Play" : "Stop")) {
		stopAddLog = !stopAddLog;
	}
	ImGui::SameLine();

	// ログの記録を初期化する
	if (ImGui::Button("Clear")) {
		logs.clear();
	}

	// ImGuiウィンドウに枠を表示する
	ImGui::BeginChild("##debuglog", ImVec2(0.0f, 0.0f), true);	
		
	// ログを出力
	for (const auto& log : logs) {
		ImGui::Text("[%.3f] ", log.first);	// 記録時間を表示（小数第3まで）
		ImGui::SameLine();
		ImGui::Text(log.second.c_str());	// ログ内容の表示
	}

	// 最新のログが常に見えるようにオートスクロールする
	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::EndChild();
	ImGui::End();
}
