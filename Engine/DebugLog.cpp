/**
* @file DebugLog.cpp
*/
#include "DebugLog.h"

#include "Log/LogBuffer.h"
#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>


namespace Debug {
	/**
	* ImGui�̃��O�E�B���h�E�ɕ\������
	*/
	void Log(const std::string message) {
		LogBuffer::GetInstance().AddLog(message);
	}
} // namespace Debug

namespace BuildDebug {
	/**
	* �����t����������f�o�b�O�E�B���h�E�ɕ\������
	*/
	void Log(Type type, const char* func, const char* format, ...)
	{
		char buffer[1024];
		char* p = buffer;
		size_t size = sizeof(buffer) - 1; // ������\n��ǉ����邽��

		// ���O�̎�ނƊ֐�����ݒ�
		static const char* const typeNames[] = { "�G���[", "�x��", "���" };
		size_t n = snprintf(p, size, "[%s] %s: ",
			typeNames[static_cast<int>(type)], func);
		p += n;
		size -= n;

		// ���b�Z�[�W��ݒ�
		va_list ap;	/*�ψ������X�g���Ǘ�����I�u�W�F�N�g�^*/
		va_start(ap, format);
		p += vsnprintf(p, size, format, ap);
		va_end(ap);	/*va_list�^�ϐ���j��*/

		//if (type != Type::info) {
		//	// �G���[��x���Ȃ�ImGuiLog�ɂ��o��
		//	Debug::Log(func);
		//}

		// �����ɉ��s��ǉ�
		p[0] = '\n';
		p[1] = '\0';

		// �쐬������������f�o�b�O�E�B���h�E�ɕ\��
		OutputDebugString(buffer);
	}
}	// namespace BuildDebug