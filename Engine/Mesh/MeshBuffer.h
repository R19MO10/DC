/**
* @file Mesh.h
*/
#ifndef MESHBUFFER_H_INCLUDED
#define MESHBUFFER_H_INCLUDED
#include "StaticMesh.h"
#include "../../Utility/MyFName.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

// ��s�錾
class MeshBuffer;
using MeshBufferPtr = std::shared_ptr<MeshBuffer>;
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

/**
* �ǂݍ���StaticMesh�����Ǘ�����N���X
*/
class MeshBuffer
{
private:
	std::shared_ptr<class MikkTSpace> mikkTSpace;	//MikkTSpace�N���X
	std::unordered_map<MyFName, StaticMeshPtr> meshes;
	PrimitiveBuffer primitiveBuffer;

private:
	std::vector<MaterialPtr> LoadMTL(
		const std::string& foldername, const char* filename);

public:
	/**
	* ���b�V���o�b�t�@���쐬����
	*
	* @param bufferSize_byte �i�[�ł��钸�_�f�[�^�̃T�C�Y(�o�C�g��)
	*/
	static MeshBufferPtr Create(size_t bufferSize)
	{
		return std::make_shared<MeshBuffer>(bufferSize);
	}

	// �R���X�g���N�^�E�f�X�g���N�^
	MeshBuffer(size_t bufferSize);
	~MeshBuffer() = default;

	// �R�s�[�Ƒ�����֎~
	MeshBuffer(const MeshBuffer&) = delete;
	MeshBuffer& operator=(const MeshBuffer&) = delete;

	/**
�@  * OBJ�t�@�C����ǂݍ���
�@  *
�@  * @param filename OBJ�t�@�C����
�@  *
�@  * @return filename����쐬�������b�V��
�@  */
	StaticMeshPtr LoadOBJ(const char* filename);

	// �S�Ă̒��_�f�[�^���폜
	void Clear();

	/**
	* �X�^�e�B�b�N���b�V���̎擾
	*
	* @param name �X�^�e�B�b�N���b�V���̖��O
	*
	* @return ���O��name�ƈ�v����X�^�e�B�b�N���b�V��
	*/
	StaticMeshPtr GetStaticMesh(const char* name) const;

	// VAO�̎擾
	VertexArrayObjectPtr GetVAO() const;

	/**
	* �����̕`��p�����[�^�ƃe�N�X�`������V�����X�^�e�B�b�N���b�V�����쐬����
	*
	* @param name         ���b�V����
	* @param params       ���b�V���̌��ɂȂ�`��p�����[�^
	* @param texBaseColor ���b�V���ɐݒ肷��x�[�X�J���[�e�N�X�`��
	*
	* @return �쐬�����X�^�e�B�b�N���b�V��
	*/
	StaticMeshPtr CreateStaticMesh(const char* name,
		const Primitive& prim, const TexturePtr& texBaseColor);

	// XY���ʂ̃v���[�g���b�V�����쐬����
	StaticMeshPtr CreatePlaneXY(const char* name);
};

/**
* �����Ă���@����₤
*
* @param vertices    ���_�z��
* @param vertexCount ���_�z��̗v�f��
* @param indices     �C���f�b�N�X�z��
* @param indexCount  �C���f�b�N�X�z��̗v�f��
*/
void FillMissingNormals(
	Vertex* vertices, size_t vertexCount,
	const uint16_t* indices, size_t indexCount);

#endif // MESHBUFFER_H_INCLUDE