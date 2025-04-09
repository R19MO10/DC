/**
* @file FramebufferObject.cpp
*/
#include "FramebufferObject.h"

#include "../DebugLog.h"

/**
* 既存のテクスチャからFBOを作成するコンストラクタ
*/
FramebufferObject::FramebufferObject(
	const TexturePtr& color, const TexturePtr& depth,
	int colorLevel, int depthLevel)
	: texColor(color), texDepth(depth)
{
	glCreateFramebuffers(1, &fbo);

	// カラーバッファを設定
	if (color) {
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, *color, colorLevel);
		width = texColor->GetWidth();
		height = texColor->GetHeight();
	}
	else {
		glNamedFramebufferDrawBuffer(fbo, GL_NONE);
	}

	// 深度バッファを設定
	if (depth) {
		glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, *depth, depthLevel);
		width = texDepth->GetWidth();
		height = texDepth->GetHeight();
	}

	// FBOのエラーチェック
	if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;
		texColor.reset();
		texDepth.reset();
		LOG_ERROR("FBOの作成に失敗");
		return;
	}

	// 作成成功
	LOG("FBO(width=%d, height=%d)を作成", width, height);
}

/**
* デストラクタ
*/
FramebufferObject::~FramebufferObject()
{
	glDeleteFramebuffers(1, &fbo);
}