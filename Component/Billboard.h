/**
* @file Billboard.h
*/
#ifndef BILLBOARD_H_INCLUDED
#define BILLBOARD_H_INCLUDED
#include "../Engine/Component.h"
#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"

/**
* �r���{�[�h
*/
class Billboard : public Component
{
public:
	Billboard(const char* name = "Billboard")
		:Component(name)
	{};
	virtual ~Billboard() = default;

	virtual void EndUpdate(
		float deltaSeconds, mat4& ownerWorldTransMat) override 
	{
		// ��ɃJ�����̕��������悤�Ƀ��b�Z�[�W�I�u�W�F�N�g�̌����𒲐�
		if (GameObject* camera = GetOwner()->GetEngine()->GetMainCameraObject()) {
			// �g�嗦�𒊏o
			const vec3 scale = ExtractScale(ownerWorldTransMat);

			// �r���{�[�h���f��̍��W�ϊ��s����쐬
			ownerWorldTransMat =
				TransformMatrix(vec3(ownerWorldTransMat[3]), camera->rotation, scale);
		}
	}
};
using BillboardPtr = std::shared_ptr<Billboard>;

#endif // BILLBOARD_H_INCLUDED