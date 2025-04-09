/**
* @file FramebufferObject.cpp
*/
#include "FramebufferObject.h"

#include "../DebugLog.h"

/**
* �����̃e�N�X�`������FBO���쐬����R���X�g���N�^
*/
FramebufferObject::FramebufferObject(
	const TexturePtr& color, const TexturePtr& depth,
	int colorLevel, int depthLevel)
	: texColor(color), texDepth(depth)
{
	glCreateFramebuffers(1, &fbo);

	// �J���[�o�b�t�@��ݒ�
	if (color) {
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, *color, colorLevel);
		width = texColor->GetWidth();
		height = texColor->GetHeight();
	}
	else {
		glNamedFramebufferDrawBuffer(fbo, GL_NONE);
	}

	// �[�x�o�b�t�@��ݒ�
	if (depth) {
		glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, *depth, depthLevel);
		width = texDepth->GetWidth();
		height = texDepth->GetHeight();
	}

	// FBO�̃G���[�`�F�b�N
	if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;
		texColor.reset();
		texDepth.reset();
		LOG_ERROR("FBO�̍쐬�Ɏ��s");
		return;
	}

	// �쐬����
	LOG("FBO(width=%d, height=%d)���쐬", width, height);
}

/**
* �f�X�g���N�^
*/
FramebufferObject::~FramebufferObject()
{
	glDeleteFramebuffers(1, &fbo);
}