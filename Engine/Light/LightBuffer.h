/**
* @file LightBuffer.h
*/
#ifndef LIGHTBUFFER_H_INCLUDE
#define LIGHTBUFFER_H_INCLUDE
#include <vector>
#include <memory>
#include <utility>
#include "../../UniformLocation.h"
#include "../../Math/MyMath.h"

// ��s�錾
class LightBuffer;
using LightBufferPtr = std::shared_ptr<LightBuffer>;
class Engine;
class MappedBufferObject;
using MappedBufferObjectPtr = std::shared_ptr<MappedBufferObject>;
class GameObject;
class Light;
using LightPtr = std::shared_ptr<Light>;

/**
* �Q�[�����̃��C�g�R���|�[�l���g���Ǘ�����N���X
*/
class LightBuffer
{
	friend Engine;
private:
	static constexpr size_t lightResizeCount = 100;	// ��x�ɑ��₷���C�g��

	std::vector<LightPtr> lights;	// ���C�g�R���|�[�l���g�z��

	// SSBO���C�g�f�[�^
	struct SSBOLightData
	{
		vec3 color = vec3(0);		// �F�Ɩ��邳
		float falloffAngle = 0;		// �X�|�b�g���C�g�̌����J�n�p�x(Spot)

		vec3 position = vec3(0);	// �ʒu
		float radius = 0;			// ���C�g���͂��ő唼�a

		vec3 direction = vec3(0);   // ���C�g�̌���(Spot)
		float coneAngle = 0;		// �X�|�b�g���C�g���Ƃ炷�p�x(Spot)
	};

	MappedBufferObjectPtr ssboLight;	// ���C�g�pSSBO

public:
	/**
	* ���C�g�o�b�t�@���쐬����
	*
	* @param bufferSize_byte �i�[�ł��钸�_�f�[�^�̃T�C�Y(�o�C�g��)
	*/
	static LightBufferPtr Create()
	{
		return std::make_shared<LightBuffer>();
	}

	// �R���X�g���N�^�E�f�X�g���N�^
	LightBuffer();
	~LightBuffer() = default;

	// �R�s�[�Ƒ�����֎~
	LightBuffer(const LightBuffer&) = delete;
	LightBuffer& operator=(const LightBuffer&) = delete;


	/**
	* �V�������C�g��ǉ�����
	*
	* @param lightPtr ���C�g�R���|�[�l���g�|�C���^
	*/
	void Allocate(const LightPtr lightPtr);


private:
	/**
	* �J�����ɋ߂����C�g��I���GPU�������ɃR�s�[����
	*
	* @param cameraWorldMat	�J�����̃��[���h���W�ϊ��s��
	*/
	void UpdateShaderLight(
		const mat4& cameraWorldMat);

	// SSBO�̃o�C���h
	void Bind();
	void Unbind();
};


#endif // LIGHTBUFFER_H_INCLUDE
