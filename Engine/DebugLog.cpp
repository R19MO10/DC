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
	* ImGuiのログウィンドウに表示する
	*/
	void Log(const std::string message) {
		LogBuffer::GetInstance().AddLog(message);
	}
} // namespace Debug

namespace BuildDebug {
	/**
	* 書式付き文字列をデバッグウィンドウに表示する
	*/
	void Log(Type type, const char* func, const char* format, ...)
	{
		char buffer[1024];
		char* p = buffer;
		size_t size = sizeof(buffer) - 1; // 末尾に\nを追加するため

		// ログの種類と関数名を設定
		static const char* const typeNames[] = { "エラー", "警告", "情報" };
		size_t n = snprintf(p, size, "[%s] %s: ",
			typeNames[static_cast<int>(type)], func);
		p += n;
		size -= n;

		// メッセージを設定
		va_list ap;	/*可変引数リストを管理するオブジェクト型*/
		va_start(ap, format);
		p += vsnprintf(p, size, format, ap);
		va_end(ap);	/*va_list型変数を破棄*/

		//if (type != Type::info) {
		//	// エラーや警告ならImGuiLogにも出す
		//	Debug::Log(func);
		//}

		// 末尾に改行を追加
		p[0] = '\n';
		p[1] = '\0';

		// 作成した文字列をデバッグウィンドウに表示
		OutputDebugString(buffer);
	}
}	// namespace BuildDebug