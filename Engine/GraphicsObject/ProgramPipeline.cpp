/**
* @file ProgramPipeline.cpp
*/
#include "ProgramPipeline.h"

#include "../DebugLog.h"
#include <fstream>
#include <filesystem>
#include <vector>


/**
* シェーダファイルを読み込んでコンパイルする
* 
* @param type		シェーダの種類
* @param filename	シェーダファイル名
* 
* @return	シェーダの管理番号
*/
GLuint CompileShader(GLenum type, std::string filename)
{
	filename = "Res/Shader/" + filename;
	std::ifstream file(filename, std::ios::binary); // ファイルを開く
	if (!file) {
		char s[256];
		snprintf(s, 256, "[ERROR] %s: %sを開けません\n", __func__, filename.c_str());
		LOG_ERROR("%sを開けません", filename.c_str());
		return 0;
	}

	// ファイルを読み込む
	const size_t filesize = std::filesystem::file_size(filename);
	//ファイルのサイズ分の領域を確保
	std::vector<char> buffer(filesize);
	file.read(buffer.data(), filesize);
	file.close();

	// ソースコードを設定してコンパイル
	const char* source[] = { buffer.data() };
	const GLint length[] = { int(buffer.size()) };
	//シェーダー管理オブジェクトの作成
	const GLuint object = glCreateShader(type);
	//シェーダをコンパイル
	glShaderSource(object, 1, source, length);
	glCompileShader(object);

	return object;
}

/**
* シェーダを読み込んでコンパイルする
*
* @param filenameVS 頂点シェーダファイル名
* @param filenameFS フラグメントシェーダファイル名
*/
ProgramPipeline::ProgramPipeline(
	const std::string& filenameVS, const std::string& filenameFS) :
	filenameVS(filenameVS), filenameFS(filenameFS)
{
	// シェーダを読み込んでコンパイル
	vs = CompileShader(GL_VERTEX_SHADER, filenameVS);
	fs = CompileShader(GL_FRAGMENT_SHADER, filenameFS);

	// 2つのシェーダをリンク
	if (vs && fs) {	//２つともコンパイルに成功したら
		pp = glCreateProgram();
		//リンクしたいシェーダーを指定
		glAttachShader(pp, vs);
		glAttachShader(pp, fs);
		//指定したシェーダをリンク
		glLinkProgram(pp);

		// リンクの成否を調べる
		GLint result;
		//リンクの状態を調べる
		glGetProgramiv(pp, GL_LINK_STATUS, &result);

		if (result != GL_TRUE) {
			LOG_ERROR("シェーダのリンクに失敗(vs=%s, fs=%s)",
				filenameVS.c_str(), filenameFS.c_str());
		}
	} // if (vs && vs)
}


// シェーダを削除
ProgramPipeline::~ProgramPipeline()
{
	glDeleteProgram(pp);
	glDeleteShader(fs);
	glDeleteShader(vs);
}