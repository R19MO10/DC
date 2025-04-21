/**
* @file SpriteRenderer.h
*/
#ifndef SPRITERENDERER_H_INCLUDED
#define SPRITERENDERER_H_INCLUDED
#include "Renderer.h"
#include "../Math/MyMath.h"
#include "../Engine/Material/Material.h"

// 先行宣言
struct StaticMesh;
using  StaticMeshPtr = std::shared_ptr<StaticMesh>;


/**
* スプライト描画コンポーネント
*/
class SpriteRenderer : public Renderer
{
private:
	StaticMeshPtr planeXYMesh;	// プレートXY

public:
	Material material;	// 表示に使うマテリアル

public:
	SpriteRenderer()
		:Renderer("SpriteRenderer")
	{
		useLighting = false;
		castShadow = false;
	};
	virtual ~SpriteRenderer() = default;

	virtual ModelFormat GetModelFormat() const override { return ModelFormat::obj; }
	
	virtual void Draw(const ProgramPipeline& prog) override;

public:
	virtual void Awake() override;
	virtual void DrawImGui() override;
};
using SpriteRendererPtr = std::shared_ptr<SpriteRenderer>;

#endif // SPRITERENDERER_H_INCLUDED