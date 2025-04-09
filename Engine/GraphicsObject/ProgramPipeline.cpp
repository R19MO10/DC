/**
* @file ProgramPipeline.cpp
*/
#include "ProgramPipeline.h"

#include "../DebugLog.h"
#include <fstream>
#include <filesystem>
#include <vector>


/**
* �V�F�[�_�t�@�C����ǂݍ���ŃR���p�C������
* 
* @param type		�V�F�[�_�̎��
* @param filename	�V�F�[�_�t�@�C����
* 
* @return	�V�F�[�_�̊Ǘ��ԍ�
*/
GLuint CompileShader(GLenum type, std::string filename)
{
	filename = "Res/Shader/" + filename;
	std::ifstream file(filename, std::ios::binary); // �t�@�C�����J��
	if (!file) {
		char s[256];
		snprintf(s, 256, "[ERROR] %s: %s���J���܂���\n", __func__, filename.c_str());
		LOG_ERROR("%s���J���܂���", filename.c_str());
		return 0;
	}

	// �t�@�C����ǂݍ���
	const size_t filesize = std::filesystem::file_size(filename);
	//�t�@�C���̃T�C�Y���̗̈���m��
	std::vector<char> buffer(filesize);
	file.read(buffer.data(), filesize);
	file.close();

	// �\�[�X�R�[�h��ݒ肵�ăR���p�C��
	const char* source[] = { buffer.data() };
	const GLint length[] = { int(buffer.size()) };
	//�V�F�[�_�[�Ǘ��I�u�W�F�N�g�̍쐬
	const GLuint object = glCreateShader(type);
	//�V�F�[�_���R���p�C��
	glShaderSource(object, 1, source, length);
	glCompileShader(object);

	return object;
}

/**
* �V�F�[�_��ǂݍ���ŃR���p�C������
*
* @param filenameVS ���_�V�F�[�_�t�@�C����
* @param filenameFS �t���O�����g�V�F�[�_�t�@�C����
*/
ProgramPipeline::ProgramPipeline(
	const std::string& filenameVS, const std::string& filenameFS) :
	filenameVS(filenameVS), filenameFS(filenameFS)
{
	// �V�F�[�_��ǂݍ���ŃR���p�C��
	vs = CompileShader(GL_VERTEX_SHADER, filenameVS);
	fs = CompileShader(GL_FRAGMENT_SHADER, filenameFS);

	// 2�̃V�F�[�_�������N
	if (vs && fs) {	//�Q�Ƃ��R���p�C���ɐ���������
		pp = glCreateProgram();
		//�����N�������V�F�[�_�[���w��
		glAttachShader(pp, vs);
		glAttachShader(pp, fs);
		//�w�肵���V�F�[�_�������N
		glLinkProgram(pp);

		// �����N�̐��ۂ𒲂ׂ�
		GLint result;
		//�����N�̏�Ԃ𒲂ׂ�
		glGetProgramiv(pp, GL_LINK_STATUS, &result);

		if (result != GL_TRUE) {
			LOG_ERROR("�V�F�[�_�̃����N�Ɏ��s(vs=%s, fs=%s)",
				filenameVS.c_str(), filenameFS.c_str());
		}
	} // if (vs && vs)
}


// �V�F�[�_���폜
ProgramPipeline::~ProgramPipeline()
{
	glDeleteProgram(pp);
	glDeleteShader(fs);
	glDeleteShader(vs);
}