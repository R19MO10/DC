/**
* @file PrimitiveBuffer.h
*/
#ifndef PRIMITIVEBUFFER_H_INCLUDED
#define PRIMITIVEBUFFER_H_INCLUDED
#include "glad/glad.h"
#include "Primitive.h"
#include "../../Math/MyMath.h"
#include <memory>
#include <vector>

// ��s�錾
class PrimitiveBuffer;
using PrimitiveBufferPtr = std::shared_ptr<PrimitiveBuffer>;
class BufferObject;
using BufferObjectPtr = std::shared_ptr<BufferObject>;
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

/**
* �v���~�e�B�u���Ǘ�����N���X
*/
class PrimitiveBuffer
{
private:
	std::vector<Primitive> primitives; // �v���~�e�B�u�z��

	VertexArrayObjectPtr vao; // ���_�A�g���r���[�g�z��
	BufferObjectPtr buffer;   // ���_�f�[�^����уC���f�b�N�X�f�[�^

	size_t usedBytes = 0;     // �o�b�t�@�̎g�p�ςݗe��(�o�C�g)

public:
	// �v���~�e�B�u�o�b�t�@���쐬����
	static PrimitiveBufferPtr Create(size_t bufferSize) {
		return std::make_shared<PrimitiveBuffer>(bufferSize);
	}

	// �R���X�g���N�^�E�f�X�g���N�^
	PrimitiveBuffer(size_t bufferSize);
	~PrimitiveBuffer() = default;

	// �R�s�[�Ƒ�����֎~
	PrimitiveBuffer(const PrimitiveBuffer&) = delete;
	PrimitiveBuffer& operator=(const PrimitiveBuffer&) = delete;

	/**
	* �v���~�e�B�u�̒ǉ�
	*
	* @param vertices    GPU�������ɃR�s�[���钸�_�f�[�^�z��
	* @param vertexBytes vertices�̃o�C�g��
	* @param indices     GPU�������ɃR�s�[����C���f�b�N�X�f�[�^�z��
	* @param indexBytes  indices�̃o�C�g��
	* @param mode        �v���~�e�B�u�̎��
	*/
	void AddPrimitive(const Vertex* vertices, size_t vertexBytes,
		const uint16_t* indices, size_t indexBytes, GLenum mode = GL_TRIANGLES);

	// �v���~�e�B�u�̎擾
	inline const Primitive& GetPrimitive(size_t index) const { return primitives[index]; }

	// �v���~�e�B�u�z��̃T�C�Y���擾
	inline const size_t GetPrimitivesCount() const { return primitives.size(); }

	// VAO�̎擾
	inline const VertexArrayObjectPtr& GetVAO() const { return vao; }

	// �v���~�e�B�u�z��̈�Ԍ���Ԃ�
	inline const Primitive& GetPrimitiveBack() const { return primitives.back(); }


	// �S�Ẵv���~�e�B�u���폜
	void Clear();
};

#endif // PRIMITIVEBUFFER_H_INCLUDED