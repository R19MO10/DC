/**
* @file ProgramPipeline.h
*/
#ifndef PROGRAMPIPELINE_H_INCLUDED
#define PROGRAMPIPELINE_H_INCLUDED
#include "glad/glad.h"
#include <memory>
#include <string>

// 先行宣言
class ProgramPipeline;
using ProgramPipelinePtr = std::shared_ptr<ProgramPipeline>;

/**
* プログラムパイプラインを管理するクラス
*/
class ProgramPipeline
{
private:
	GLuint vs = 0;			// 頂点シェーダ
	GLuint fs = 0;			// フラグメントシェーダ
	GLuint pp = 0;			// プログラムパイプライン
	std::string filenameVS; // 頂点シェーダファイル名
	std::string filenameFS; // フラグメントシェーダファイル名

public:
	/**
	*　プログラムパイプラインを作成
	* 
	* @param filenameVS 頂点シェーダファイル名
	* @param filenameFS フラグメントシェーダファイル名
	* 
	* @return 作成したプログラムパイプラインへのポインタ
	*/
	static ProgramPipelinePtr Create(
		const std::string& filenameVS, const std::string& filenameFS)
	{
		return std::make_shared<ProgramPipeline>(filenameVS, filenameFS);
	}

public:
	/**
	* シェーダを読み込んでコンパイルする
	* 
	* @param filenameVS 頂点シェーダファイル名
	* @param filenameFS フラグメントシェーダファイル名
	*/
	ProgramPipeline(
		const std::string& filenameVS, const std::string& filenameFS);

	// シェーダを削除
	~ProgramPipeline();

	// コピーと代入を禁止
	ProgramPipeline(const ProgramPipeline&) = delete;
	ProgramPipeline& operator=(const ProgramPipeline&) = delete;

	// 管理番号を取得
	operator GLuint() const { return pp; }

	// 頂点シェーダの管理番号を取得
	GLuint GetVS() const { return vs; }

	// フラグメントシェーダの管理番号を取得
	GLuint GetFS() const { return fs; }
};

#endif // PROGRAMPIPELINE_H_INCLUDED