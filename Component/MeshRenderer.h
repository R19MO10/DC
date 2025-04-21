/**
* @file MeshRenderer.h
*/
#ifndef MESH_RENDERER_H_INCLUDED
#define MESH_RENDERER_H_INCLUDED
#include "Renderer.h"
#include "../Math/MyMath.h"

// ��s�錾
struct StaticMesh;
using  StaticMeshPtr = std::shared_ptr<StaticMesh>;
class Texture;
using TexturePtr = std::shared_ptr<Texture>;
struct Material;
using  MaterialPtr = std::shared_ptr<Material>;
using  MaterialList = std::vector<MaterialPtr>;


/**
* ���b�V���`��R���|�[�l���g
*/
class MeshRenderer : public Renderer
{
public:
	StaticMeshPtr	staticMesh;	// �\������X�^�e�B�b�N���b�V��
	
	TexturePtr	 texColor;	// �\���Ɏg���J���[�e�N�X�`��
	MaterialList materials; // �Q�[���I�u�W�F�N�g�ŗL�̃}�e���A���z��

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