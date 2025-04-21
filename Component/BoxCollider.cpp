/**
* @file  BoxCollider.cpp
*/
#include "BoxCollider.h"

#include "../UniformLocation.h"

#include "../Engine/Engine.h"
#include "../Engine/Mesh/StaticMesh.h"
#include "../Engine/GraphicsObject/ProgramPipeline.h"


void BoxCollider::DrawCollision(const ProgramPipeline& prog)
{
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	const mat4 transMat = { 
		{ box.axis[0] * box.scale.x,	0 },
		{ box.axis[1] * box.scale.y,	0 },
		{ box.axis[2] * box.scale.z,	0 },
		{ box.position,					1 } 
	};

	const mat4 m = owner->GetWorldTransformMatrix() * transMat;
	glProgramUniformMatrix4fv(prog, Loc::TransformMatrix, 1, GL_FALSE, &m[0].x);

	// コライダーを表示するときのマテリアルを作成
	MaterialList materials(1, std::make_shared<Material>());
	materials[0]->baseColor = engine->GetCollisionColor(*this);

	DrawStaticMesh(*engine->GetStaticMesh("Box.obj"), prog, vec4(1), materials);
}

void BoxCollider::DrawImGui()
{
	ImGui::Checkbox("Static", &isStatic);
	ImGui::Checkbox("Trigger", &isTrigger);
	ImGui::Spacing();

	ImGui::DragFloat3("Position", &box.position.x);
	ImGui::Spacing();

	if (ImGui::TreeNode("Axis")) {
		ImGui::DragFloat3("X", &box.axis[0].x);
		ImGui::DragFloat3("Y", &box.axis[1].x);
		ImGui::DragFloat3("Z", &box.axis[2].x);
		ImGui::Spacing();

		ImGui::TreePop();
	}

	ImGui::DragFloat3("Scale", &box.scale.x);
}
