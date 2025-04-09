/**
* @ file GltfMesh.h
*/
#ifndef GLTFMESH_H_INCLUDED
#define GLTFMESH_H_INCLUDED
#include "../../Utility/MyFName.h"
#include "../../Math/MyMath.h"
#include <vector>
#include <string>
#include <memory>

// ��s�錾
class Texture;
using TexturePtr = std::shared_ptr<Texture>;
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;
struct GltfFile;
using  GltfFilePtr = std::shared_ptr<GltfFile>;


/**
* �v���~�e�B�u�f�[�^
*/
struct GltfPrimitive
{
	GLenum mode = GL_TRIANGLES; // �v���~�e�B�u�̎��
	GLsizei count = 0;          // �`�悷��C���f�b�N�X��
	GLenum type = GL_UNSIGNED_SHORT; // �C���f�b�N�X�f�[�^�^
	const GLvoid* indices = 0;  // �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g
	GLint baseVertex = 0;       // �C���f�b�N�X0�ԂƂ݂Ȃ���钸�_�z����̈ʒu

	VertexArrayObjectPtr vao;    // �v���~�e�B�u�pVAO
	size_t materialNo = 0; // �}�e���A���ԍ�
};

/**
* ���b�V���f�[�^
*/
struct GltfMesh
{
	MyFName name; // ���b�V����
	std::vector<GltfPrimitive> primitives; // �v���~�e�B�u�z��
};

#endif // GLTFMESH_H_INCLUDED