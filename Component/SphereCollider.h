/**
* @file ShpereCollider.h
*/
#ifndef SPHERECOLLIDER_H_INCLUDED
#define SPHERECOLLIDER_H_INCLUDED
#include "Collider.h"

/**
* 球体コライダー
*/
class SphereCollider : public Collider
{
public:
	Sphere sphere = { vec3(0), 1 }; // 図形(球体)

public:
	SphereCollider() 
		:Collider("SphereCollider")
	{};
	virtual ~SphereCollider() = default;

	// 図形の種類
	inline Type GetType() const override { return Type::Sphere; }

	// 座標を変更する
	inline void AddPosition(const vec3& translate) override {
		sphere.position += translate;
	}

	// 座標変換したコライダーを取得する
	inline ColliderPtr GetTransformedCollider(const mat4& transform) const override {
		// 中心座標を座標変換する
		const auto& p = std::make_shared<SphereCollider>();
		p->sphere.position = vec3(transform * vec4(sphere.position, 1));

		// 球体の拡大率はオブジェクトのXYZ拡大率のうち最大のものとする(Unity準拠)
		const vec3 scale = ExtractScale(transform);
		const float maxScale = std::max({ scale.x, scale.y, scale.z });
		p->sphere.radius = sphere.radius * maxScale;

		return p;
	}

	// 図形を取得する
	inline const Sphere& GetShape() const { return sphere; }

public:
	// コリジョンを表示する
	virtual void DrawCollision(const ProgramPipeline& prog);

public:
	virtual void DrawImGui() override;
};
using SphereColliderPtr = std::shared_ptr<SphereCollider>;

#endif // SPHERECOLLIDER_H_INCLUDED