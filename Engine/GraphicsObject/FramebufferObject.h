/**
* @file FramebufferObject.h
*/
#ifndef FRAMEBUFFEROBJECT_H_INCLUDED
#define FRAMEBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include "../Texture/Texture.h"

// 先行宣言
class FramebufferObject;
using FramebufferObjectPtr = std::shared_ptr<FramebufferObject>;

/**
* フレームバッファ・オブジェクト(FBO)
*/
class FramebufferObject
{
private:
	GLuint fbo = 0;      // FBOの管理番号
	int width = 0;       // FBOの幅
	int height = 0;      // FBOの高さ
	TexturePtr texColor; // カラーテクスチャ
	TexturePtr texDepth; // 深度テクスチャ

public:
	/**
	*　テクスチャからFBOを作成
	*
	* @param color カラーテクスチャ
	* @param depth 深度テクスチャ
	* @param colorLevel カラーテクスチャのミップレベル
	* @param depthLevel 深度テクスチャのミップレベル
	*
	* @return 作成したフレームバッファ・オブジェクトへのポインタ
	*/
	static FramebufferObjectPtr Create(const TexturePtr& color, const TexturePtr& depth,
		int colorLevel = 0, int depthLevel = 0) 
	{
		return std::make_shared<FramebufferObject>(color, depth, colorLevel, depthLevel);
	}

	/**
	* テクスチャからFBOを作成するコンストラクタ
	*
	* @param color カラーテクスチャ
	* @param depth 深度テクスチャ
	* @param colorLevel カラーテクスチャのミップレベル
	* @param depthLevel 深度テクスチャのミップレベル
	*/
	FramebufferObject(const TexturePtr& color, const TexturePtr& depth,
		int colorLevel = 0, int depthLevel = 0);
	~FramebufferObject();

	// コピーと代入を禁止
	FramebufferObject(const FramebufferObject&) = delete;
	FramebufferObject& operator=(const FramebufferObject&) = delete;

	// 管理番号を取得
	operator GLuint() const { return fbo; }

	// 幅と高さを取得
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

	// テクスチャを取得
	const TexturePtr& GetColorTexture() const { return texColor; }
	const TexturePtr& GetDepthTexture() const { return texDepth; }
};

#endif // FRAMEBUFFEROBJECT_H_INCLUDED