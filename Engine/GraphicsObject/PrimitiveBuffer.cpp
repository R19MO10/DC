/**
* @file PrimitiveBuffer.cpp
*/
#include "PrimitiveBuffer.h"

#include "../DebugLog.h"
#include "BufferObject.h"
#include "VertexArrayObject.h"
#include <numeric>


PrimitiveBuffer::PrimitiveBuffer(size_t bufferSize)
{
	// �o�b�t�@�I�u�W�F�N�g���쐬
	buffer = BufferObject::Create(bufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);

	// VAO���쐬
	vao = VertexArrayObject::Create();

	// VBO��IBO��VAO�Ƀo�C���h
	glBindVertexArray(*vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);

	// ���_�A�g���r���[�g��ݒ�
	vao->SetAttribute(0, 3, sizeof(Vertex), offsetof(Vertex, position));
	vao->SetAttribute(1, 2, sizeof(Vertex), offsetof(Vertex, texcoord));
	vao->SetAttribute(2, 3, sizeof(Vertex), offsetof(Vertex, normal));
	vao->SetAttribute(3, 4, sizeof(Vertex), offsetof(Vertex, tangent));

	// ��������삪�s���Ȃ��悤�Ƀo�C���h������
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// �`��p�����[�^�̗e�ʂ�\��
	primitives.reserve(100);
}


void PrimitiveBuffer::AddPrimitive(const Vertex* vertices, size_t vertexBytes,
	const uint16_t* indices, size_t indexBytes, GLenum mode)
{
	// �󂫗e�ʂ�����Ă��邱�Ƃ��m�F
	// �K�v�ȃo�C�g�����󂫃o�C�g�����傫���ꍇ�͒ǉ��ł��Ȃ�
	const size_t totalBytes = vertexBytes + indexBytes;
	const size_t freeBytes = buffer->GetSize() - usedBytes;
	if (totalBytes > freeBytes) {
		LOG_ERROR("�e�ʂ�����܂���(�v���T�C�Y=%d, �g�p��/�ő�e��=%d/%d)",
			vertexBytes + indexBytes, usedBytes, buffer->GetSize());
		return;
	}

	// ���_�f�[�^��GPU�������ɃR�s�[�iVBO��IBO�ɓ����o�b�t�@�I�u�W�F�N�g���w�肵�Ă���j
	GLuint tmp[2];
	glCreateBuffers(2, tmp);	/*�ꎞ�I�ȃo�b�t�@���쐬*/
	glNamedBufferStorage(tmp[0], vertexBytes, vertices, 0);	/*�ꎞ�o�b�t�@�ɏ������݁i���_�f�[�^�p�o�b�t�@�j*/
	glNamedBufferStorage(tmp[1], indexBytes, indices, 0);	/*�ꎞ�o�b�t�@�ɏ������݁i�C���f�b�N�X�f�[�^�p�o�b�t�@�j*/

	//�R�s�[
	/*(�R�s�[���o�b�t�@�̊Ǘ��ԍ�, �R�s�[��o�b�t�@�̊Ǘ��ԍ�, �R�s�[���̓ǂݎ��J�n�ʒu, �R�s�[��̏������݊J�n�ʒu, �R�s�[����o�C�g��);*/
	glCopyNamedBufferSubData(tmp[0], *buffer, 0, usedBytes, vertexBytes);
	glCopyNamedBufferSubData(tmp[1], *buffer, 0, usedBytes + vertexBytes, indexBytes);

	glDeleteBuffers(2, tmp);	/*�ꎞ�o�b�t�@���폜*/


	// �ǉ������}�`�̕`��p�����[�^���쐬
	Primitive newPrim;
	newPrim.mode = mode;
	newPrim.count = static_cast<GLsizei>(indexBytes / sizeof(uint16_t));
	// �C���f�b�N�X�f�[�^�̈ʒu(�o�C�g���Ŏw��)�͒��_�f�[�^�̒���
	newPrim.indices = reinterpret_cast<void*>(usedBytes + vertexBytes);
	// ���_�f�[�^�̈ʒu�͒��_�f�[�^���Ŏw�肷��
	newPrim.baseVertex = static_cast<GLint>(usedBytes / sizeof(Vertex));

	primitives.push_back(newPrim);	 // �`��p�����[�^��z��ɒǉ�

	// ���̃f�[�^�i�[�J�n�ʒu���v�Z�i�C���f�b�N�X�f�[�^�ƒ��_�f�[�^�̍ŏ���{��(20byte)�j
	constexpr size_t a = std::lcm(sizeof(uint16_t)/*�C���f�b�N�X�f�[�^(2byte)*/, sizeof(Vertex)/*���_�f�[�^(20byte)*/); // ���ʂ̋��E�T�C�Y

	//�u���̒l�ȏォ�A�ŏ��̋��E�T�C�Y�̔{���v
	usedBytes += ((totalBytes + a - 1) / a) * a;	/*((���̒l + ���E�T�C�Y - 1) / ���E�T�C�Y) * ���E�T�C�Y*/
}

/**
* �S�Ẵv���~�e�B�u���폜����
*/
void PrimitiveBuffer::Clear()
{
	primitives.clear();
	usedBytes = 0;
}