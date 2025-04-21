/**
* @file  SphereCollider.cpp
*/
#include "SphereCollider.h"

#include "../UniformLocation.h"

#include "../Engine/Engine.h"
#include "../Engine/Mesh/StaticMesh.h"
#include "../Engine/GraphicsObject/ProgramPipeline.h"


void SphereCollider::DrawCollision(const ProgramPipeline& prog)
{
	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	const mat4 transMat = TransformMatrix(sphere.position, vec3(0), vec3(sphere.radius));

	const mat4 m = owner->GetWorldTransformMatrix() * transMat;
	glProgramUniformMatrix4fv(prog, Loc::TransformMatrix, 1, GL_FALSE, &m[0].x);

	// コライダーを表示するときのマテリアルを作成
	MaterialList materials(1, std::make_shared<Material>());
	materials[0]->baseColor = engine->GetCollisionColor(*this);

	DrawStaticMesh(*engine->GetStaticMesh("Sphere.obj"), prog, vec4(1), materials);
}

void SphereCollider::DrawImGui()
{
	ImGui::Checkbox("Static", &isStatic);
	ImGui::Checkbox("Trigger", &isTrigger);
	ImGui::Spacing();

	ImGui::DragFloat3("Posiotion", &sphere.position.x);
	ImGui::DragFloat("Radius", &sphere.radius);
}
