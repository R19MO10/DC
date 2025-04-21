/**
* @file SpriteRenderer.cpp
*/
#include "SpriteRenderer.h"

#include "../UniformLocation.h"

#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"
#include "../Engine/Mesh/MeshBuffer.h"
#include "../Engine/GraphicsObject/ProgramPipeline.h"


void SpriteRenderer::Draw(const ProgramPipeline& prog)
{
	if (planeXYMesh) {
		GameObject* owner = GetOwner();

		/*���b�V�������_����ZYX��]���Q�[���I�u�W�F�N�g��ZYX��]*/
		const mat4 matModel =
			owner->GetWorldTransformMatrix() * 
			TransformMatrix(translate, rotation, scale);
		glProgramUniformMatrix4fv(prog, Loc::TransformMatrix,
			1, GL_FALSE, &matModel[0].x);

		DrawSpritePlaneMesh(*planeXYMesh, prog, owner->color, material);
	}
}

void SpriteRenderer::Awake()
{
	// �v���[�g���b�V�����Z�b�g
	Engine* engine = GetOwner()->GetEngine();
	planeXYMesh = engine->GetStaticMesh("PlaneXY");
}

void SpriteRenderer::DrawImGui()
{
	Renderer::DrawImGui();
	ImGui::Spacing();

	if (ImGui::TreeNode("Material")) {
		// �J���[
		if (material.texBaseColor) {
			ImGui::Text(material.texBaseColor->GetName().c_str());
			ImGui::ColorEdit4("Base Color", &material.baseColor.x);
			ImGui::Spacing();
		}

		// �G�~�b�V����
		if (material.texEmission) {
			ImGui::Text(material.texEmission->GetName().c_str());
		}
		ImGui::ColorEdit3("Emission Color", &material.emission.x);
		ImGui::SliderFloat("Emission Strength", &material.emission.w, 0, 100);

		ImGui::TreePop();
	}
}