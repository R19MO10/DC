/**
* @file MeshRenderer.h
*/
#ifndef MESH_RENDERER_H_INCLUDED
#define MESH_RENDERER_H_INCLUDED
#include "Renderer.h"
#include "../Math/MyMath.h"

// 先行宣言
struct StaticMesh;
using  StaticMeshPtr = std::shared_ptr<StaticMesh>;
class Texture;
using TexturePtr = std::shared_ptr<Texture>;
struct Material;
using  MaterialPtr = std::shared_ptr<Material>;
using  MaterialList = std::vector<MaterialPtr>;


/**
* メッシュ描画コンポーネント
*/
class MeshRenderer : public Renderer
{
public:
	StaticMeshPtr	staticMesh;	// 表示するスタティックメッシュ
	
	TexturePtr	 texColor;	// 表示に使うカラーテクスチャ
	MaterialList materials; // ゲームオブジェクト固有のマテリアル配列

public:
	MeshRenderer()
		:Renderer("MeshRenderer")
	{};
	virtual ~MeshRenderer() = default;

	virtual ModelFormat GetModelFormat() const override { return ModelFormat::obj; }

	virtual void Draw(const ProgramPipeline& prog) override;

public:
	virtual void DrawImGui() override;
};
using MeshRendererPtr = std::shared_ptr<MeshRenderer>;

#endif // MESH_RENDERER_H_INCLUDED