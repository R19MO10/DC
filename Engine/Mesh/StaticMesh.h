/**
* @file StaticMesh.h
*/
#ifndef STATICMESH_H_INCLUDED
#define STATICMESH_H_INCLUDED
#include "../Material/Material.h"
#include "../GraphicsObject/PrimitiveBuffer.h"

/**
* 3Dモデル
*/
struct StaticMesh
{
	std::string name;    // メッシュ名
	std::vector<Primitive> primitives; // 描画パラメータ
	MaterialList materials; // 共有マテリアル配列
};
using StaticMeshPtr = std::shared_ptr<StaticMesh>;
void DrawStaticMesh(
	const StaticMesh& mesh, GLuint program, 
	const vec4& objectColor, const MaterialList& materials);
void DrawSpritePlaneMesh(
	const StaticMesh& planeMesh, GLuint program, 
	const vec4& objectColor, const Material& material);

#endif // ! STATICMESH_H_INCLUDED