/**
* @file AabbCollider.cpp
*/
#include "AabbCollider.h"

#include "../UniformLocation.h"

#include "../Engine/Engine.h"
#include "../Engine/Mesh/StaticMesh.h"
#include "../Engine/GraphicsObject/ProgramPipeline.h"


void AabbCollider::DrawCollision(const ProgramPipeline& prog)
{
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	const mat4 transMat = TransformMatrix(aabb.position, vec3(0), aabb.max);

	const mat4 m = owner->GetWorldTransformMatrix() * transMat;
	glProgramUniformMatrix4fv(prog, Loc::TransformMatrix, 1, GL_FALSE, &m[0].x);

	// コライダーを表示するときのマテリアルを作成
	MaterialList materials(1, std::make_shared<Material>());
	materials[0]->baseColor = engine->GetCollisionColor(*this);

	DrawStaticMesh(*engine->GetStaticMesh("Box.obj"), prog, vec4(1), materials);
}

void AabbCollider::DrawImGui()
{
	ImGui::Checkbox("Static", &isStatic);
	ImGui::Checkbox("Trigger", &isTrigger);
	ImGui::Spacing();

	ImGui::DragFloat3("Position", &aabb.position.x);
	ImGui::Spacing();

	ImGui::DragFloat3("Min", &aabb.min.x);
	ImGui::DragFloat3("Max", &aabb.max.x);
}
