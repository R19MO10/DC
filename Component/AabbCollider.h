/**
* @file AabbCollider.h
*/
#ifndef AABBCOLLIDER_H_INCLUDED
#define AABBCOLLIDER_H_INCLUDED
#include "Collider.h"

/**
* AABBコライダー
*/
class AabbCollider : public Collider
{
public:
	AABB aabb = {
		{  0,  0,  0 },
		{ -1, -1, -1 }, 
		{  1,  1,  1 } 
	}; // 図形(軸平行境界ボックス)

public:
	AabbCollider()
		:Collider("AabbCollider")
	{};
	virtual ~AabbCollider() = default;

	// 図形の種類
	inline Type GetType() const override { return Type::AABB; }

	// 座標を変更する
	inline void AddPosition(const vec3& translate) override {
		aabb.position += translate;
	}

	/**
	* 座標変換したコライダーを取得する
	*
	* 回転角度は90度単位で指定すること。それ以外の角度では正しい交差判定が行えない。
	*/
	inline ColliderPtr GetTransformedCollider(const mat4& transform) const override {
		// 座標変換したコピーを作成
		const auto& p = std::make_shared<AabbCollider>();
		p->aabb.min = vec3(transform * vec4(aabb.min + aabb.position, 1));
		p->aabb.max = vec3(transform * vec4(aabb.max + aabb.position, 1));

		// minのほうが大きかったら入れ替える:
		for (int i = 0; i < 3; ++i) {
			if (p->aabb.min[i] > p->aabb.max[i]) {
				const float tmp = p->aabb.min[i];
				p->aabb.min[i] = p->aabb.max[i];
				p->aabb.max[i] = tmp;
			}
		}
		return p;
	}

	// 図形を取得する
	inline const AABB& GetShape() const { return aabb; }

public:
	// コリジョンを表示する
	virtual void DrawCollision(const ProgramPipeline& prog) override;
	
public:
	virtual void DrawImGui() override;
};
using AabbColliderPtr = std::shared_ptr<AabbCollider>;

#endif // AABBCOLLIDER_H_INCLUDED