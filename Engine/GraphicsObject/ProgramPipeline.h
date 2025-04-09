/**
* @file ProgramPipeline.h
*/
#ifndef PROGRAMPIPELINE_H_INCLUDED
#define PROGRAMPIPELINE_H_INCLUDED
#include "glad/glad.h"
#include <memory>
#include <string>

// ��s�錾
class ProgramPipeline;
using ProgramPipelinePtr = std::shared_ptr<ProgramPipeline>;

/**
* �v���O�����p�C�v���C�����Ǘ�����N���X
*/
class ProgramPipeline
{
private:
	GLuint vs = 0;			// ���_�V�F�[�_
	GLuint fs = 0;			// �t���O�����g�V�F�[�_
	GLuint pp = 0;			// �v���O�����p�C�v���C��
	std::string filenameVS; // ���_�V�F�[�_�t�@�C����
	std::string filenameFS; // �t���O�����g�V�F�[�_�t�@�C����

public:
	/**
	*�@�v���O�����p�C�v���C�����쐬
	* 
	* @param filenameVS ���_�V�F�[�_�t�@�C����
	* @param filenameFS �t���O�����g�V�F�[�_�t�@�C����
	* 
	* @return �쐬�����v���O�����p�C�v���C���ւ̃|�C���^
	*/
	static ProgramPipelinePtr Create(
		const std::string& filenameVS, const std::string& filenameFS)
	{
		return std::make_shared<ProgramPipeline>(filenameVS, filenameFS);
	}

public:
	/**
	* �V�F�[�_��ǂݍ���ŃR���p�C������
	* 
	* @param filenameVS ���_�V�F�[�_�t�@�C����
	* @param filenameFS �t���O�����g�V�F�[�_�t�@�C����
	*/
	ProgramPipeline(
		const std::string& filenameVS, const std::string& filenameFS);

	// �V�F�[�_���폜
	~ProgramPipeline();

	// �R�s�[�Ƒ�����֎~
	ProgramPipeline(const ProgramPipeline&) = delete;
	ProgramPipeline& operator=(const ProgramPipeline&) = delete;

	// �Ǘ��ԍ����擾
	operator GLuint() const { return pp; }

	// ���_�V�F�[�_�̊Ǘ��ԍ����擾
	GLuint GetVS() const { return vs; }

	// �t���O�����g�V�F�[�_�̊Ǘ��ԍ����擾
	GLuint GetFS() const { return fs; }
};

#endif // PROGRAMPIPELINE_H_INCLUDED