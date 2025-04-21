/**
* @file MeshRenderer.cpp
*/
#include "MeshRenderer.h"

#include "../UniformLocation.h"

#include "../Engine/GameObject.h"
#include "../Engine/Mesh/MeshBuffer.h"
#include "../Engine/GraphicsObject/ProgramPipeline.h"


void MeshRenderer::Draw(const ProgramPipeline& prog)
{
	if (staticMesh) {
		// �`��Ɏg���e�N�X�`�����w��
		if (texColor) {
			const GLuint tex = *texColor;
			glBindTextures(Bind::Texture::Color, 1, &tex);
		}
		
		GameObject* owner = GetOwner();

		/* ���b�V�������_����ZYX��]���Q�[���I�u�W�F�N�g��ZYX��] */
		const mat4 matModel = 
			owner->GetWorldTransformMatrix() * 
			TransformMatrix(translate, rotation, scale);
		glProgramUniformMatrix4fv(prog, Loc::TransformMatrix,
			1, GL_FALSE, &matModel[0].x);

		if (materials.empty()) {
			DrawStaticMesh(*staticMesh, prog, owner->color, staticMesh->materials);
		}
		else {
			DrawStaticMesh(*staticMesh, prog, owner->color, materials);
		}
	}
}

void MeshRenderer::DrawImGui()
{
	Renderer::DrawImGui();
	ImGui::Spacing();

	ImGui::Text("StaticMesh");
	if (staticMesh) {
		ImGui::Text("	%s", staticMesh->name.c_str());
	}
	ImGui::Spacing();

	ImGui::Text("TexColor");
	if (texColor) {
		ImGui::Text("	%s", texColor->GetName().c_str());
	}
	ImGui::Spacing();

	if (ImGui::TreeNode("Materials")) {
		if (materials.empty()) {
			if (ImGui::Button("Create")) {
				if (staticMesh) {
					materials = CloneMaterialList(staticMesh);
				}
			}
		}
		else {
			for (const auto& m : materials) {
				m->DrawImGui();
			}
			ImGui::Spacing();

			if (ImGui::Button("Delete")) {
				materials.clear();
			}
		}
		ImGui::TreePop();
	}
}
