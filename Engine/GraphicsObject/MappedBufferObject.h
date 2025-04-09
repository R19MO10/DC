/**
* @file MappedBufferObject.h
*/
#ifndef MAPPEDBUFFEROBJECT_H_INCLUDED
#define MAPPEDBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include <memory>

// ��s�錾
class MappedBufferObject;
using MappedBufferObjectPtr = std::shared_ptr<MappedBufferObject>;

// SSBO�̃T�C�Y���v�Z����
inline size_t CalcSsboSize(size_t n)
{
	const size_t alignment = 256; // �A���C�����g(�f�[�^�̋��E)
	return ((n + alignment - 1) / alignment) * alignment;
};

/**
* �o�b�t�@�I�u�W�F�N�g(�}�b�v����)
*/
class MappedBufferObject
{
private:
	GLuint id = 0;
	GLsizeiptr bufferSize = 0;

	uint8_t* p = nullptr;          // �f�[�^�R�s�[��A�h���X
	size_t bufferIndex = 0;        // �R�s�[��o�b�t�@�̃C���f�b�N�X
	GLsync syncList[2] = { 0, 0 }; // �����I�u�W�F�N�g
	GLenum type = GL_ARRAY_BUFFER; // �o�b�t�@�̎��

public:
	/**
	*�@�o�b�t�@�I�u�W�F�N�g���쐬����
	*
	* @param size        �o�b�t�@�I�u�W�F�N�g�̃o�C�g�T�C�Y
	* @param type        �o�b�t�@�I�u�W�F�N�g�̎��
	* @param access      �ȉ��̂����ꂩ���w�肷��
	*                    GL_READ_WRITE �ǂݎ��Ə������݂̗��Ή��Ń}�b�v����
	*                    GL_READ_ONLY  �ǂݎ���p�Ƃ��ă}�b�v����
	*                    GL_WRITE_ONLY �������ݐ�p�Ƃ��ă}�b�v����
	*/
	static MappedBufferObjectPtr Create(GLsizeiptr size, GLenum type, GLenum access)
	{
		return std::make_shared<MappedBufferObject>(size, type, access);
	}

	MappedBufferObject(
		GLsizeiptr size, GLenum type, GLenum access);
	~MappedBufferObject();

	operator GLuint() const { return id; }
	GLsizeiptr GetSize() const { return bufferSize; }


	// GPU���o�b�t�@���g���I���̂�҂�
	void WaitSync();

	// �f�[�^�̃R�s�[��A�h���X���擾����
	uint8_t* GetMappedAddress() const;

	// �w�肵���͈͂�OpenGL�R���e�L�X�g�Ɋ��蓖�Ă�
	void Bind(GLuint index, size_t offset, size_t size);

	// �R�s�[��o�b�t�@��؂�ւ���
	void SwapBuffers();

	GLenum GetType() const { return type; }
};

#endif // MAPPEDBUFFEROBJECT_H_INCLUDED