/**
* @file FramebufferObject.h
*/
#ifndef FRAMEBUFFEROBJECT_H_INCLUDED
#define FRAMEBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include "../Texture/Texture.h"

// ��s�錾
class FramebufferObject;
using FramebufferObjectPtr = std::shared_ptr<FramebufferObject>;

/**
* �t���[���o�b�t�@�E�I�u�W�F�N�g(FBO)
*/
class FramebufferObject
{
private:
	GLuint fbo = 0;      // FBO�̊Ǘ��ԍ�
	int width = 0;       // FBO�̕�
	int height = 0;      // FBO�̍���
	TexturePtr texColor; // �J���[�e�N�X�`��
	TexturePtr texDepth; // �[�x�e�N�X�`��

public:
	/**
	*�@�e�N�X�`������FBO���쐬
	*
	* @param color �J���[�e�N�X�`��
	* @param depth �[�x�e�N�X�`��
	* @param colorLevel �J���[�e�N�X�`���̃~�b�v���x��
	* @param depthLevel �[�x�e�N�X�`���̃~�b�v���x��
	*
	* @return �쐬�����t���[���o�b�t�@�E�I�u�W�F�N�g�ւ̃|�C���^
	*/
	static FramebufferObjectPtr Create(const TexturePtr& color, const TexturePtr& depth,
		int colorLevel = 0, int depthLevel = 0) 
	{
		return std::make_shared<FramebufferObject>(color, depth, colorLevel, depthLevel);
	}

	/**
	* �e�N�X�`������FBO���쐬����R���X�g���N�^
	*
	* @param color �J���[�e�N�X�`��
	* @param depth �[�x�e�N�X�`��
	* @param colorLevel �J���[�e�N�X�`���̃~�b�v���x��
	* @param depthLevel �[�x�e�N�X�`���̃~�b�v���x��
	*/
	FramebufferObject(const TexturePtr& color, const TexturePtr& depth,
		int colorLevel = 0, int depthLevel = 0);
	~FramebufferObject();

	// �R�s�[�Ƒ�����֎~
	FramebufferObject(const FramebufferObject&) = delete;
	FramebufferObject& operator=(const FramebufferObject&) = delete;

	// �Ǘ��ԍ����擾
	operator GLuint() const { return fbo; }

	// ���ƍ������擾
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

	// �e�N�X�`�����擾
	const TexturePtr& GetColorTexture() const { return texColor; }
	const TexturePtr& GetDepthTexture() const { return texDepth; }
};

#endif // FRAMEBUFFEROBJECT_H_INCLUDED