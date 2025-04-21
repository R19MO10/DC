/**
* @file Material.cpp
*/
#include "Material.h"

#include <imgui.h>
#include "../Mesh/StaticMesh.h"
#include "../Gltf/GltfFile.h"


/**
* マテリアルの詳細を表示する
*/
void Material::DrawImGui()
{
	if (ImGui::TreeNode(name.c_str())) {
		// カラー
		if (texBaseColor) {
			ImGui::Text(texBaseColor->GetName().c_str());
		}
		if (texNormal) {
			ImGui::Text(texNormal->GetName().c_str());
		}
		if (texEmission) {
			ImGui::Text(texEmission->GetName().c_str());
		}
		ImGui::Spacing();

		ImGui::ColorEdit4("Base Color", &baseColor.x);
		ImGui::Spacing();

		ImGui::ColorEdit3("Emission Color", &emission.x);
		ImGui::SliderFloat("Emission Strength", &emission.w, 0, 100);
		ImGui::Spacing();

		ImGui::SliderFloat("Roughness", &roughness, 0.00001f, 1.0f);
		roughness = std::max(roughness, 0.00001f);
		ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);

		ImGui::TreePop();
	}
}


/**
* 共有マテリアル配列を複製する
*
* @param original 複製するマテリアルを持つファイル
*
* @return 複製してマテリアル配列
*/
MaterialList CloneMaterialList(const StaticMeshPtr& original)
{
	MaterialList clone(original->materials.size());
	for (int i = 0; i < clone.size(); ++i) {
		clone[i] = std::make_shared<Material>(*original->materials[i]);
	}
	return clone;
}
MaterialList CloneMaterialList(const GltfFilePtr& original)
{
	MaterialList clone(original->materials.size());
	for (int i = 0; i < clone.size(); ++i) {
		clone[i] = std::make_shared<Material>(*original->materials[i]);
	}
	return clone;
}
