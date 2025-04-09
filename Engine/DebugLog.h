/**
* @file DebugLog.h
*/
#ifndef DEBUGLOG_H_INCLUDED
#define DEBUGLOG_H_INCLUDED
#include "../Utility/MyFName.h"
#include "../Math/MyMath.h"
#include <string>

// ファイルパスからファイル名のみを取得する
inline std::string GetFileName(const std::string& fullPath) {
	// ファイル名の開始位置を取得
	size_t lastSlash = fullPath.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		// ファイル名の部分を抽出して返す
		return fullPath.substr(lastSlash + 1) + ": ";
	}

	// パスにディレクトリ区切り文字が含まれていない場合は、そのまま返す
	return fullPath + ": ";
}

/**
* デバッグ用の機能を格納する名前空間
*/
namespace Debug {
	// ImGuiのログウィンドウに表示する
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
	// メッセージの種類
	enum class Type {
		error,   // エラー
		warning, // 警告
		info,    // その他の情報
	};

	void Log(Type type, const char* func, const char* format, ...);
}

#ifndef NDEBUG	/*Debugビルド時のみ有効にする*/
#   define ENABLE_DEBUG_LOG // このマクロを定義するとデバッグ出力が有効になる
#endif // NDEBUG

// デバッグ出力マクロ
#ifdef ENABLE_DEBUG_LOG
#   define LOG_ERROR(...)   BuildDebug::Log(BuildDebug::Type::error, __func__, __VA_ARGS__)	/*実行に支障をきたすレベルの問題*/
#   define LOG_WARNING(...) BuildDebug::Log(BuildDebug::Type::warning, __func__, __VA_ARGS__)	/*設定ミスなどの軽微な問題*/
#   define LOG(...)         BuildDebug::Log(BuildDebug::Type::info, __func__, __VA_ARGS__)	/*情報として出力したいメッセージ*/
#else
#   define LOG_ERROR(...)   ((void)0)
#   define LOG_WARNING(...) ((void)0)
#   define LOG(...)         ((void)0)
#endif // ENABLE_DEBUG_LOG

#endif // DEBUG_H_INCLUDED