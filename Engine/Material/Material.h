/**
* @ file Material.h
*/
#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED
#include "../../Math/MyMath.h"
#include "../Texture/Texture.h"
#include <vector>

// ��s�錾
struct StaticMesh;
using StaticMeshPtr = std::shared_ptr<StaticMesh>;
struct GltfFile;
using GltfFilePtr = std::shared_ptr<GltfFile>;

/**
* �}�e���A��
*/
struct Material
{
	std::string name = "<Default>";  // �}�e���A����
	vec4 baseColor = vec4(1);	// ��{�F+�A���t�@
	vec4 emission = vec4(0);    // �����F
	float roughness = 0.7f;     // �\�ʂ̑e��(0.0�`1.0)
	float metallic = 0.0f;      // 0=����� 1=����

	TexturePtr texBaseColor;    // ��{�F�e�N�X�`��
	TexturePtr texEmission;     // �����F�e�N�X�`��
	TexturePtr texNormal;       // �@���e�N�X�`��

	// �}�e���A���̏��\��
	void DrawImGui();
};

using MaterialPtr = std::shared_ptr<Material>;
using MaterialList = std::vector<MaterialPtr>; // �}�e���A���z��^

// ���L�}�e���A���z��𕡐�����
MaterialList CloneMaterialList(const StaticMeshPtr& original);
MaterialList CloneMaterialList(const GltfFilePtr& original);

#endif // MATERIAL_H_INCLUDED