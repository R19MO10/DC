/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include "glad/glad.h"
#include "../../Utility/MyFName.h"
#include <string>
#include <memory>

// 先行宣言
class Texture;
using TexturePtr = std::shared_ptr<Texture>;

// テクスチャの利用目的
enum class Usage
{
	for2D, // 2D描画用
	for3D, // 3D描画用
	forGltf, // glTF描画用
};

/**
* テクスチャ管理クラス
*/
class Texture
{
	friend class TextureManager;
private:
	MyFName name;	// テクスチャ名
	GLuint id = 0;  // オブジェクト管理番号
	int width = 0;  // テクスチャの幅
	int height = 0; // テクスチャの高さ

private:
	/**
	* コンストラクタ（テクスチャを読み込む）
	*
	* @param filename テクスチャファイル名
	* @param usage    利用目的
	*/
	explicit Texture(const char* filename, Usage usage = Usage::for3D);

	/**
	* 空のテクスチャを作成するコンストラクタ
	*
	* @param name      テクスチャ識別用の名前
	* @param width     テクスチャの幅(ピクセル数)
	* @param heightOffset    テクスチャの高さ(ピクセル数)
	* @param gpuFormat データ形式
	* @param wrapMode  ラップモード
	* @param levels    ミップマップテクスチャのレベル数
	*/
	Texture(const char* name, int width, int height,
		GLenum gpuFormat, GLenum wrapMode = GL_CLAMP_TO_EDGE, int levels = 1);

	~Texture();

	// コピーと代入を禁止
	/*同じ管理番号を持つTextureクラスができてしまうと同じ管理番号に対して
	【glDeleteTextures】が複数回呼ばれてしまうため*/
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

public:
	// 名前を取得
	inline const std::string& GetName() const { return name.GetName(); }

	// 管理番号を取得
	inline operator GLuint() const { return id; }

	// 幅と高さを取得
	inline int GetWidth() const { return width; }
	inline int GetHeight() const { return height; }

	// 画像のアスペクト比を取得（アスペクト比 = 画像の幅 ÷ 画像の高さ）
	inline float GetAspectRatio() const {
		return static_cast<float>(width) / static_cast<float>(height);
	}

	/**
	* ラップモードを設定
	*
	* @param wrapMode 設定するラップモード
	*/
	void SetWrapMode(GLenum wrapMode);
};
#endif // TEXTURE_H_INCLUDED