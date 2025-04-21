/**
* @file LightBuffer.cpp
*/
#include "LightBuffer.h"

#include "../GameObject.h"
#include "../GraphicsObject/ProgramPipeline.h"
#include "../GraphicsObject/MappedBufferObject.h"
#include "../../Component/Light.h"
#include <GLFW/glfw3.h>
#include <algorithm>


/**
* �R���X�g���N�^
*/
LightBuffer::LightBuffer()
{
	// ���C�g�z���\��
	lights.reserve(lightResizeCount);

	// SSBO���쐬
	ssboLight = MappedBufferObject::Create(
		MaxShaderLightCount * sizeof(SSBOLightData),
		GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
}

/**
* �V�������C�g��ǉ�����
*
* @param lightPtr ���C�g�R���|�[�l���g�|�C���^
*/
void LightBuffer::Allocate(const LightPtr lightPtr)
{
	lights.push_back(lightPtr);
}


/**
* �J�����ɋ߂����C�g��I���GPU�������ɃR�s�[����
*
* @param cameraWorldMat	�J�����̃��[���h���W�ϊ��s��
*/
void LightBuffer::UpdateShaderLight(
	const mat4& cameraWorldMat)
{
	// �R�s�[���郉�C�g���Ȃ���΃��C�g����0�ɐݒ�
	if (lights.empty()) {
		ssboLight->WaitSync();
		uint8_t* p = ssboLight->GetMappedAddress();
		const int lightCount[4] = { 0 };
		memcpy(p, lightCount, sizeof(lightCount));
		return;
	}

	// ���C�g�R���|�[�l���g���폜���ꂽ��z�񂩂���폜����
	const auto& unused_itr = std::remove_if(lights.begin(), lights.end(),
		[](const LightPtr lightPtr) { return lightPtr->IsDestroyed(); });
	lights.erase(unused_itr, lights.end());

	// �J�����̍��W
	const vec3 cameraPos = vec3(cameraWorldMat[3]);
	// �J�����̐��ʃx�N�g��
	const vec3 cameraForward = { 
		cameraWorldMat[2].x,
		0,
		cameraWorldMat[2].z,
	};


	// �J�������烉�C�g�܂ł̋������v�Z
	std::vector<std::pair<float, const Light*>> distanceList;
	distanceList.reserve(lights.size());

	for (const auto& e : lights) {
		const auto& light = e;
		const GameObject* lightOwner = light->GetOwner();

		// �����󋵂𒲂ׂ�
		if (!lightOwner->CheckActive()) {
			continue;
		}
		if (!light->isActive) {
			continue;
		}

		// ���C�g�̖��邳��0�ȉ��Ȃ��΂�
		if (light->intensity <= 0) {
			continue;
		}

		// ���C�g�̈ʒu���擾
		const vec3 lightPos = vec3(lightOwner->GetWorldTransformMatrix()[3]);

		// �J�������烉�C�g�ւ̃x�N�g��
		const vec3 CameraToLightDirection = lightPos - cameraPos;

		// �J�����̌��ɂ��郉�C�g�����O
		if (dot(cameraForward, CameraToLightDirection) <= -light->radius) {
			continue;
		}

		// �J�������烉�C�g�̔��a�܂ł̋���
		const float d = length(CameraToLightDirection) - light->radius; 
		distanceList.push_back(std::make_pair(d, light.get()));
	}

	// ��ʂɉe�����郉�C�g���Ȃ���΃��C�g����0�ɐݒ�
	if (distanceList.empty()) {
		ssboLight->WaitSync();
		uint8_t* p = ssboLight->GetMappedAddress();
		const int lightCount[4] = { 0 };
		memcpy(p, lightCount, sizeof(lightCount));
		return;
	}

	// �J�����ɋ߂����C�g��D�悷��
	std::stable_sort(distanceList.begin(), distanceList.end(),
		[&](const auto& a, const auto& b) { return a.first < b.first; });


	// ���C�g�f�[�^��v�f���Ƃɂ܂Ƃ߂�GPU�������ɃR�s�[
	const int lightCount = static_cast<int>(
		std::min(distanceList.size(), MaxShaderLightCount)); // �R�s�[���郉�C�g��

	// �R�s�[���郉�C�g�z����쐬
	std::vector<SSBOLightData> copyList(lightCount);
	for (int i = 0; i < lightCount; ++i) {
		const Light* e = distanceList[i].second;
		const mat4 lightOwnerWorldMat = e->GetOwner()->GetWorldTransformMatrix();

		SSBOLightData data;
		data.position = vec3(lightOwnerWorldMat[3]);
		data.color = e->color * e->intensity;
		data.radius = e->radius;

		data.direction = vec3(0);
		data.coneAngle = 0;	// �Ǝˊp�x��0�ȉ��Ȃ�|�C���g���C�g����
		data.falloffAngle = 0;

		// �X�|�b�g���C�g�̏ꍇ�A�X�|�b�g���C�g�̃p�����[�^�𔽉f
		if (e->type == Light::Type::SpotLight) {
			data.direction = normalize(vec3(lightOwnerWorldMat[2]));
			data.coneAngle = e->coneAngle_rad;
			data.falloffAngle = e->falloffAngle_rad;
		}

		copyList[i] = data;
	}

	// ���C�g�f�[�^��GPU�������ɃR�s�[
	ssboLight->WaitSync();
	uint8_t* p = ssboLight->GetMappedAddress();

	const int cnt[4] = { lightCount };
	memcpy(p, cnt, sizeof(cnt));
	p += sizeof(cnt);
	memcpy(p, copyList.data(), copyList.size() * sizeof(SSBOLightData));
}

/**
* SSBO���o�C���h����
*/
void LightBuffer::Bind()
{
	ssboLight->Bind(Bind::SSBO::Light, 0, ssboLight->GetSize());
}

/**
* SSBO�̃o�C���h����������
*/
void LightBuffer::Unbind()
{
	glBindBufferRange(
		ssboLight->GetType(), Bind::SSBO::Light, 0, 0, 0);
	ssboLight->SwapBuffers();
}
