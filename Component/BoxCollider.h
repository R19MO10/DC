/**
* @file BoxCollider.h
*/
#ifndef BOXCOLLIDER_H_INCLUDED
#define BOXCOLLIDER_H_INCLUDED
#include "Collider.h"

/**
* BOX(OBB)コライダー
*/
class BoxCollider : public Collider
{
public:
	Box box = {
		{ 0, 0, 0 },
		{ { 1, 0, 0}, { 0, 1, 0 }, { 0, 0, 1} },
		{ 1, 1, 1 } 
	};

public:
	BoxCollider() 
		:Collider("BoxCollider")
	{};
	virtual ~BoxCollider() = default;

	// 図形の種類
	inline Type GetType() const override { return Type::Box; }

	// 座標を変更する
	inline void AddPosition(const vec3& translate) override
	{
		box.position += translate;
	}

	// 座標変換したコライダーを取得する
	inline ColliderPtr GetTransformedCollider(const mat4& transform) const override
	{
		// 座標変換行列を分解
		vec3 translate; // 未使用
		mat3 rotation;
		vec3 scale;
		Decompose(transform, translate, rotation, scale);
		
		// 座標変換したコピーを作成
		const auto& p = std::make_shared<BoxCollider>();
		p->box.position = vec3(transform * vec4(box.position, 1));
		for (int i = 0; i < 3; ++i) {
			p->box.axis[i]  = rotation * box.axis[i];	// 軸ベクトル
			p->box.scale[i] = scale[i] * box.scale[i];	// 拡大率
		}
		return p;
	}

	// 図形を取得する
	inline const Box& GetShape() const { return box; }

public:
	// コリジョンを表示する
	virtual void DrawCollision(const ProgramPipeline& prog);

public:
	virtual void DrawImGui() override;
};
using BoxColliderPtr = std::shared_ptr<BoxCollider>;

#endif // BOXCOLLIDER_H_INCLUDED