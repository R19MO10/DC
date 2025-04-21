/**
* @file Collider.h
*/
#ifndef COLLIDER_H_INCLUDED
#define COLLIDER_H_INCLUDED
#include "../Engine/Component.h"
#include "../Engine/Collision/Collision.h"
#include "../Math/MyMath.h"

// 先行宣言
class ProgramPipeline;

/**
* コライダーの基底クラス
*/
class Collider : public Component
{
public:
	CollisionType type = CollisionType::Default;
	int priority = 0;		// 衝突優先度(値が高い方が小さい方と当たった時に押されない)
	bool isTrigger = false; // true=重複を許可, false=重複を禁止
	bool isStatic = false;  // true=動かない物体, false=動く物体

public:
	Collider(const char* compName = "Collider")
		:Component(compName)
	{};
	virtual ~Collider() = default;

public:
	// 図形の種類
	enum class Type {
		AABB,   // 軸平行境界ボックス
		Sphere, // 球体
		Box,    // 有向境界ボックス
	};
	virtual Type GetType() const = 0;

	// 座標を変更する
	virtual void AddPosition(const vec3& translate) = 0;

	// 座標変換したコライダーを取得する
	virtual ColliderPtr GetTransformedCollider(const mat4& transform) const = 0;

public:
	// コリジョンを描画(可視化表示)する
	virtual void DrawCollision(const ProgramPipeline& prog) = 0;
};
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;
using ColliderList = std::vector<ColliderPtr>;

#endif // COLLIDER_H_INCLUDED