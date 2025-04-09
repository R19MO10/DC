/**
* @file DebugLog.h
*/
#ifndef DEBUGLOG_H_INCLUDED
#define DEBUGLOG_H_INCLUDED
#include "../Utility/MyFName.h"
#include "../Math/MyMath.h"
#include <string>

// �t�@�C���p�X����t�@�C�����݂̂��擾����
inline std::string GetFileName(const std::string& fullPath) {
	// �t�@�C�����̊J�n�ʒu���擾
	size_t lastSlash = fullPath.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		// �t�@�C�����̕����𒊏o���ĕԂ�
		return fullPath.substr(lastSlash + 1) + ": ";
	}

	// �p�X�Ƀf�B���N�g����؂蕶�����܂܂�Ă��Ȃ��ꍇ�́A���̂܂ܕԂ�
	return fullPath + ": ";
}

/**
* �f�o�b�O�p�̋@�\���i�[���閼�O���
*/
namespace Debug {
	// ImGui�̃��O�E�B���h�E�ɕ\������
	void Log(const std::string message);

	inline void Log(const MyFName message) { Log(message.GetName()); }
	inline void Log(const char*   message) { Log(std::string(message)); }
	inline void Log(const int	  message) { Log(std::to_string(message)); }
	inline void Log(const float	  message) { Log(std::to_string(message)); }
	inline void Log(const double  message) { Log(std::to_string(message)); }
	inline void Log(const size_t  message) { Log(static_cast<int>(message)); }
	inline void Log(const bool	  message) {
		std::string str = (message == true) ? "true" : "false";
		Log(str);
	}
	inline void Log(const vec2	  message) {
		std::string str =
			"(" + std::to_string(message.x) + ", " + std::to_string(message.y) + ")";
		Log(str);
	}
	inline void Log(const vec3	  message) {
		std::string str =
			"(" + std::to_string(message.x) + ", " + std::to_string(message.y) + ", "
			+ std::to_string(message.z) + ")";
		Log(str);
	}
	inline void Log(const vec4	  message) {
		std::string str =
			"(" + std::to_string(message.x) + ", " + std::to_string(message.y) + ", "
			+ std::to_string(message.z) + ", " + std::to_string(message.w) + ")";
		Log(str);
	}
}

namespace BuildDebug {
	// ���b�Z�[�W�̎��
	enum class Type {
		error,   // �G���[
		warning, // �x��
		info,    // ���̑��̏��
	};

	void Log(Type type, const char* func, const char* format, ...);
}

#ifndef NDEBUG	/*Debug�r���h���̂ݗL���ɂ���*/
#   define ENABLE_DEBUG_LOG // ���̃}�N�����`����ƃf�o�b�O�o�͂��L���ɂȂ�
#endif // NDEBUG

// �f�o�b�O�o�̓}�N��
#ifdef ENABLE_DEBUG_LOG
#   define LOG_ERROR(...)   BuildDebug::Log(BuildDebug::Type::error, __func__, __VA_ARGS__)	/*���s�Ɏx������������x���̖��*/
#   define LOG_WARNING(...) BuildDebug::Log(BuildDebug::Type::warning, __func__, __VA_ARGS__)	/*�ݒ�~�X�Ȃǂ̌y���Ȗ��*/
#   define LOG(...)         BuildDebug::Log(BuildDebug::Type::info, __func__, __VA_ARGS__)	/*���Ƃ��ďo�͂��������b�Z�[�W*/
#else
#   define LOG_ERROR(...)   ((void)0)
#   define LOG_WARNING(...) ((void)0)
#   define LOG(...)         ((void)0)
#endif // ENABLE_DEBUG_LOG

#endif // DEBUG_H_INCLUDED