/**
* @ file Material.h
*/
#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED
#include "../../Math/MyMath.h"
#include "../Texture/Texture.h"
#include <vector>

// 先行宣言
struct StaticMesh;
using StaticMeshPtr = std::shared_ptr<StaticMesh>;
struct GltfFile;
using GltfFilePtr = std::shared_ptr<GltfFile>;

/**
* マテリアル
*/
struct Material
{
	std::string name = "<Default>";  // マテリアル名
	vec4 baseColor = vec4(1);	// 基本色+アルファ
	vec4 emission = vec4(0);    // 発光色
	float roughness = 0.7f;     // 表面の粗さ(0.0～1.0)
	float metallic = 0.0f;      // 0=非金属 1=金属

	TexturePtr texBaseColor;    // 基本色テクスチャ
	TexturePtr texEmission;     // 発光色テクスチャ
	TexturePtr texNormal;       // 法線テクスチャ

	// マテリアルの情報表示
	void DrawImGui();
};

using MaterialPtr = std::shared_ptr<Material>;
using MaterialList = std::vector<MaterialPtr>; // マテリアル配列型

// 共有マテリアル配列を複製する
MaterialList CloneMaterialList(const StaticMeshPtr& original);
MaterialList CloneMaterialList(const GltfFilePtr& original);

#endif // MATERIAL_H_INCLUDED