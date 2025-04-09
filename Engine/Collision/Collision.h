/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include "ContactPoint.h"

/**
* コリジョンの種類
*/
enum class CollisionType : uint8_t {
	Default,
	Stage,
	BlockingVolume,
	PlayerBody,
	DragonBody,
	DragonStoneHitBody,
	FallStone,
};

/**
* 衝突判定を行わないレイヤーの組み合わせ
*/
using CollisionTypePair = std::pair<CollisionType, CollisionType>;
constexpr CollisionTypePair NonCollidablePairs[] = {
	{	CollisionType::DragonBody,			CollisionType::Stage				},
	{	CollisionType::DragonBody,			CollisionType::BlockingVolume		},
	{	CollisionType::DragonBody,			CollisionType::DragonBody			},
	{	CollisionType::DragonBody,			CollisionType::DragonStoneHitBody	},
	{	CollisionType::DragonBody,			CollisionType::FallStone			},

	{	CollisionType::DragonStoneHitBody,	CollisionType::Stage				},
	{	CollisionType::DragonStoneHitBody,	CollisionType::BlockingVolume		},
	{	CollisionType::DragonStoneHitBody,	CollisionType::PlayerBody			},
	{	CollisionType::DragonStoneHitBody,	CollisionType::DragonBody			},
	{	CollisionType::DragonStoneHitBody,	CollisionType::DragonStoneHitBody	},

	{	CollisionType::FallStone,			CollisionType::FallStone			},
};


/**
* 軸平行境界ボックス
*/
struct AABB
{
	vec3 position; // 中心座標
	vec3 min;
	vec3 max;
};

/**
* 球体
*/
struct Sphere
{
	vec3 position; // 中心座標
	float radius;  // 半径
};

/**
* 有向境界ボックス(OBB)
*/
struct Box
{
	vec3 position; // 中心座標
	mat3 axis;  // 軸の向きベクトル
	vec3 scale;    // 拡大率
};


/**
* 線分
*/
struct LineSegment
{
	vec3 start; // 線分の始点
	vec3 end;   // 線分の終点
};

/**
* 平面
*/
struct Plane
{
	vec3 normal; // 面の法線
	float d; // 原点からの距離
};

/**
* 3D長方形（OBBの任意の軸の大きさがゼロになったもの）
*/
struct Rectangle
{
	vec3 center;  // 長方形の中心座標
	vec3 normal;  // 面の法線（外積を避けるために追加）
	vec3 axis[2]; // 長方形の軸
	float scale[2];        // 軸方向の大きさ
};


namespace Calc {
	vec3 ClosestPoint(const AABB& aabb, const vec3& p);
	vec3 ClosestPoint(const Box& box,	const vec3& p);
	vec3 ClosestPoint(const LineSegment& seg, const vec3& p);
	LineSegment ClosestPoint(const LineSegment& segA,	const LineSegment& segB);
	LineSegment ClosestPoint(const Rectangle& rect,		const LineSegment& seg);
}

bool Intersect(const AABB& aabbA,		const AABB& aabbB,		ContactPoint& cp);
bool Intersect(const AABB& aabb,		const Sphere& sphere,	ContactPoint& cp);
bool Intersect(const AABB& aabb,		const Box& box,			ContactPoint& cp);

bool Intersect(const Sphere& sphereA,	const Sphere& sphereB,	ContactPoint& cp);
bool Intersect(const Sphere& sphere,	const AABB& aabb,		ContactPoint& cp);
bool Intersect(const Sphere& sphere,	const Box& box,			ContactPoint& cp);

bool Intersect(const Box& boxA,			const Box& boxB,		ContactPoint& cp);
bool Intersect(const Box& box,			const Sphere& sphere,	ContactPoint& cp);
bool Intersect(const Box& box,			const AABB& aabb,		ContactPoint& cp);

#endif // COLLISION_H_INCLUDED