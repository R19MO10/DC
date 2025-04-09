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
* ファイル
*/
struct GltfFile
{
	MyFName name; // ファイル名
	std::vector<GltfScene> scenes; // シーン配列
	std::vector<GltfNode> nodes;   // ノード配列
	std::vector<GltfSkin> skins;   // スキン配列
	std::vector<GltfMesh> meshes;  // メッシュ配列
	MaterialList materials;   // マテリアル配列

	std::vector<GltfAnimationPtr> animations; // アニメーション
	mat4 matRoot = mat4(1); // 基本姿勢行列
};
using GltfFilePtr = std::shared_ptr<GltfFile>;
void DrawGltf(
	const GltfMesh& mesh, GLuint program,
	const vec4& objectColor, const MaterialList& materials);

#endif // GLTFFILE_H_INCLUDED