/**
* @ file GltfFile.h
*/
#ifndef GLTFFILE_H_INCLUDED
#define GLTFFILE_H_INCLUDED
#include "glad/glad.h"
#include "../Material/Material.h"
#include "GltfMesh.h"
#include "GltfAnimation.h"

/**
* �t�@�C��
*/
struct GltfFile
{
	MyFName name; // �t�@�C����
	std::vector<GltfScene> scenes; // �V�[���z��
	std::vector<GltfNode> nodes;   // �m�[�h�z��
	std::vector<GltfSkin> skins;   // �X�L���z��
	std::vector<GltfMesh> meshes;  // ���b�V���z��
	MaterialList materials;   // �}�e���A���z��

	std::vector<GltfAnimationPtr> animations; // �A�j���[�V����
	mat4 matRoot = mat4(1); // ��{�p���s��
};
using GltfFilePtr = std::shared_ptr<GltfFile>;
void DrawGltf(
	const GltfMesh& mesh, GLuint program,
	const vec4& objectColor, const MaterialList& materials);

#endif // GLTFFILE_H_INCLUDED