/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include "glad/glad.h"
#include "../../Utility/MyFName.h"
#include <string>
#include <memory>

// ��s�錾
class Texture;
using TexturePtr = std::shared_ptr<Texture>;

// �e�N�X�`���̗��p�ړI
enum class Usage
{
	for2D, // 2D�`��p
	for3D, // 3D�`��p
	forGltf, // glTF�`��p
};

/**
* �e�N�X�`���Ǘ��N���X
*/
class Texture
{
	friend class TextureManager;
private:
	MyFName name;	// �e�N�X�`����
	GLuint id = 0;  // �I�u�W�F�N�g�Ǘ��ԍ�
	int width = 0;  // �e�N�X�`���̕�
	int height = 0; // �e�N�X�`���̍���

private:
	/**
	* �R���X�g���N�^�i�e�N�X�`����ǂݍ��ށj
	*
	* @param filename �e�N�X�`���t�@�C����
	* @param usage    ���p�ړI
	*/
	explicit Texture(const char* filename, Usage usage = Usage::for3D);

	/**
	* ��̃e�N�X�`�����쐬����R���X�g���N�^
	*
	* @param name      �e�N�X�`�����ʗp�̖��O
	* @param width     �e�N�X�`���̕�(�s�N�Z����)
	* @param heightOffset    �e�N�X�`���̍���(�s�N�Z����)
	* @param gpuFormat �f�[�^�`��
	* @param wrapMode  ���b�v���[�h
	* @param levels    �~�b�v�}�b�v�e�N�X�`���̃��x����
	*/
	Texture(const char* name, int width, int height,
		GLenum gpuFormat, GLenum wrapMode = GL_CLAMP_TO_EDGE, int levels = 1);

	~Texture();

	// �R�s�[�Ƒ�����֎~
	/*�����Ǘ��ԍ�������Texture�N���X���ł��Ă��܂��Ɠ����Ǘ��ԍ��ɑ΂���
	�yglDeleteTextures�z��������Ă΂�Ă��܂�����*/
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

public:
	// ���O���擾
	inline const std::string& GetName() const { return name.GetName(); }

	// �Ǘ��ԍ����擾
	inline operator GLuint() const { return id; }

	// ���ƍ������擾
	inline int GetWidth() const { return width; }
	inline int GetHeight() const { return height; }

	// �摜�̃A�X�y�N�g����擾�i�A�X�y�N�g�� = �摜�̕� �� �摜�̍����j
	inline float GetAspectRatio() const {
		return static_cast<float>(width) / static_cast<float>(height);
	}

	/**
	* ���b�v���[�h��ݒ�
	*
	* @param wrapMode �ݒ肷�郉�b�v���[�h
	*/
	void SetWrapMode(GLenum wrapMode);
};
#endif // TEXTURE_H_INCLUDED