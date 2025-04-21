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
* �o�͂������O���o�́A�ۑ�����
*/
class LogBuffer : public Singleton<LogBuffer>
{
private:
	bool stopAddLog = false;	// ���O�̐V�K�ǉ����~����

	std::vector<std::pair<float, std::string>> logs;

public:
	LogBuffer() {
		logs.reserve(100);
	}
	virtual ~LogBuffer() = default;

	// ���O��ǉ�����
	void AddLog(const std::string& message);

	// ���O��S��ImGui�E�B���h�E�ɕ\������
	void DrawImGui_AllLogs();
};

#endif // LOGMANAGER_H_INCLUDED