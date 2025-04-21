/**
* @file LogBuffer.h
*/
#ifndef LOGMANAGER_H_INCLUDED
#define LOGMANAGER_H_INCLUDED
#include "../../Utility/Singleton.h"
#include "../../Math/MyMath.h"
#include <vector>
#include <utility>
#include <string>

/**
* 出力したログを出力、保存する
*/
class LogBuffer : public Singleton<LogBuffer>
{
private:
	bool stopAddLog = false;	// ログの新規追加を停止する

	std::vector<std::pair<float, std::string>> logs;

public:
	LogBuffer() {
		logs.reserve(100);
	}
	virtual ~LogBuffer() = default;

	// ログを追加する
	void AddLog(const std::string& message);

	// ログを全てImGuiウィンドウに表示する
	void DrawImGui_AllLogs();
};

#endif // LOGMANAGER_H_INCLUDED