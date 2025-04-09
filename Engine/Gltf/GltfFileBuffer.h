/**
* @file GltfFileBuffer.h
*/
#ifndef GLTFFILEBUFFER_H_INCLUDED
#define GLTFFILEBUFFER_H_INCLUDED
#include "GltfFile.h"
#include <unordered_map>

// ��s�錾
class GltfFileBuffer;
using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;
class BufferObject;
using BufferObjectPtr = std::shared_ptr<BufferObject>;
class MappedBufferObject;
using MappedBufferObjectPtr = std::shared_ptr<MappedBufferObject>;


// ���b�V���`��Ɏg��SSBO�͈̔�
struct GltfFileRange {
	GLintptr offset;
	size_t size;
};

/**
* glTF�t�@�C������ǂݍ��񂾃��b�V�����Ǘ�����N���X
*/
class GltfFileBuffer
{
private:
	BufferObjectPtr buffer;         // bin�t�@�C����ǂݍ��ރo�b�t�@
	uint8_t* pBuffer = nullptr;     // �}�b�v���ꂽ�A�h���X
	GLsizeiptr curBufferSize = 0;   // �������ݍς݃f�[�^��

	// �t�@�C���Ǘ��p�̘A�z�z��
	std::unordered_map<MyFName, GltfFilePtr> files;

	// �p���s��̔z��
	using AnimationMatrices = std::vector<mat4>;

	// �A�j���[�V�����̎p���s��o�b�t�@
	MappedBufferObjectPtr animationBuffer;
	AnimationMatrices tmpAnimationBuffer;

public:
	/**
	* glTF�t�@�C���o�b�t�@���쐬����
	* 
	* @param bufferCapacity �t�@�C���i�[�p�o�b�t�@�̍ő�o�C�g��
	* @param maxMatrixCount �A�j���[�V�����pSSBO�Ɋi�[�ł���ő�s��
	*/
	static GltfFileBufferPtr Create(
		size_t bufferCapacity, size_t maxMatrixCount) {
		return std::make_shared<GltfFileBuffer>(bufferCapacity, maxMatrixCount);
	}

	// �R���X�g���N�^�E�f�X�g���N�^
	GltfFileBuffer(size_t bufferCapacity, size_t maxMatrixCount);
	~GltfFileBuffer() = default;

	// �t�@�C���̓ǂݍ���
	GltfFilePtr LoadGltf(const char* filename);


	// glTF�t�@�C�����擾����
	GltfFilePtr GetGltfFile(const char* name) const;


	// �A�j���[�V�����̎p���s��o�b�t�@�̊Ǘ�
	void ClearAnimationBuffer();
	GltfFileRange AddAnimationMatrices(const AnimationMatrices& matBones);
	void UploadAnimationBuffer();
	void BindAnimationBuffer(GLuint bindingPoint, const GltfFileRange& range);
	void UnbindAnimationBuffer(GLuint bindingPoint);

private:
	GltfFilePtr Parse(const char* text, const char* foldername);
};

#endif // GLTFFILEBUFFER_H_INCLUDED