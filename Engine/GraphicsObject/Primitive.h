/**
* @file Primitive.h
*/
#ifndef PRIMITIVE_INCLUDE_H
#define PRIMITIVE_INCLUDE_H

#include "glad/glad.h"
#include "../../Math/MyMath.h"

/**
* ���_�f�[�^�`��
*/
struct Vertex
{
	vec3 position;	// ���_���W
	vec2 texcoord;	// �e�N�X�`�����W
	vec3 normal;	// �@��
	vec4 tangent;	// �^���W�F���g
};

/**
* �v���~�e�B�u
*/
struct Primitive
{
	GLenum mode = GL_TRIANGLES; // �v���~�e�B�u�̎��
	GLsizei count = 0;          // �`�悷��C���f�b�N�X��
	const void* indices = 0;    // �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g
	GLint baseVertex = 0;       // �C���f�b�N�X0�ƂȂ钸�_�z����̈ʒu
	int materialNo = -1;        // �}�e���A��
};

/**
* �v���~�e�B�u��`�悷��
*/
inline void DrawPrimitive(const Primitive& prim)
{
	glDrawElementsInstancedBaseVertex(prim.mode, prim.count,
		GL_UNSIGNED_SHORT, prim.indices, 1, prim.baseVertex);
}

#endif // !PRIMITIVE_INCLUDE_H