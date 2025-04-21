/**
* @ file Renderer.h
*/
#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED
#include "../Engine/Component.h"

// ��s�錾
class Renderer;
using RendererPtr = std::shared_ptr<Renderer>;
class ProgramPipeline;

/**
* ���f���`��
*/
enum class ModelFormat
{
	obj,          // OBJ�t�@�C��
	gltfStatic,   // glTF(�A�j���[�V�����Ȃ�)
	gltfAnimated, // glTF(�A�j���[�V��������)
};

/**
* �`��R���|�[�l���g
*/
class Renderer : public Component
{
public:
	vec3 translate = vec3(0);	// ���s�ړ�
	vec3 rotation = vec3(0);	// ��]
	vec3 scale = vec3(1);		// �g�嗦

	bool useLighting = true;	// ���̉e�����󂯂邩
	bool castShadow = true;		// �e�𗎂Ƃ���

public:
	Renderer(const char* name = "Renderer")
		: Component(name)
	{};
	virtual ~Renderer() = default;

	// ���f���`�����擾
	virtual ModelFormat GetModelFormat() const = 0;

	
	// �`��O����
	virtual void PreDraw(float deltaSeconds) {}

	// �`�揈��
	virtual void Draw(const ProgramPipeline& prog) = 0;

public:
	virtual void DrawImGui() override {
		ImGui::DragFloat3("Translate", &translate.x);
		vec3 r = radians_to_degrees(rotation);
		ImGui::DragFloat3("Rotation", &r.x);
		rotation = degrees_to_radians(r);
		ImGui::DragFloat3("Scale", &scale.x);
		ImGui::Spacing();

		ImGui::Checkbox("UseLighting", &useLighting);
		ImGui::Checkbox("CastShadow", &castShadow);
	}
};
#endif // RENDERER_H_INCLUDED
