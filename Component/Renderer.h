/**
* @ file Renderer.h
*/
#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED
#include "../Engine/Component.h"

// 先行宣言
class Renderer;
using RendererPtr = std::shared_ptr<Renderer>;
class ProgramPipeline;

/**
* モデル形式
*/
enum class ModelFormat
{
	obj,          // OBJファイル
	gltfStatic,   // glTF(アニメーションなし)
	gltfAnimated, // glTF(アニメーションあり)
};

/**
* 描画コンポーネント
*/
class Renderer : public Component
{
public:
	vec3 translate = vec3(0);	// 平行移動
	vec3 rotation = vec3(0);	// 回転
	vec3 scale = vec3(1);		// 拡大率

	bool useLighting = true;	// 光の影響を受けるか
	bool castShadow = true;		// 影を落とすか

public:
	Renderer(const char* name = "Renderer")
		: Component(name)
	{};
	virtual ~Renderer() = default;

	// モデル形式を取得
	virtual ModelFormat GetModelFormat() const = 0;

	
	// 描画前処理
	virtual void PreDraw(float deltaSeconds) {}

	// 描画処理
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
