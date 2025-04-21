/**
* @file SpriteRenderer.h
*/
#ifndef SPRITERENDERER_H_INCLUDED
#define SPRITERENDERER_H_INCLUDED
#include "Renderer.h"
#include "../Math/MyMath.h"
#include "../Engine/Material/Material.h"

// ��s�錾
struct StaticMesh;
using  StaticMeshPtr = std::shared_ptr<StaticMesh>;


/**
* �X�v���C�g�`��R���|�[�l���g
*/
class SpriteRenderer : public Renderer
{
private:
	StaticMeshPtr planeXYMesh;	// �v���[�gXY

public:
	Material material;	// �\���Ɏg���}�e���A��

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