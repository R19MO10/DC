/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include "ContactPoint.h"

/**
* �R���W�����̎��
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
* �Փ˔�����s��Ȃ����C���[�̑g�ݍ��킹
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
* �����s���E�{�b�N�X
*/
struct AABB
{
	vec3 position; // ���S���W
	vec3 min;
	vec3 max;
};

/**
* ����
*/
struct Sphere
{
	vec3 position; // ���S���W
	float radius;  // ���a
};

/**
* �L�����E�{�b�N�X(OBB)
*/
struct Box
{
	vec3 position; // ���S���W
	mat3 axis;  // ���̌����x�N�g��
	vec3 scale;    // �g�嗦
};


/**
* ����
*/
struct LineSegment
{
	vec3 start; // �����̎n�_
	vec3 end;   // �����̏I�_
};

/**
* ����
*/
struct Plane
{
	vec3 normal; // �ʂ̖@��
	float d; // ���_����̋���
};

/**
* 3D�����`�iOBB�̔C�ӂ̎��̑傫�����[���ɂȂ������́j
*/
struct Rectangle
{
	vec3 center;  // �����`�̒��S���W
	vec3 normal;  // �ʂ̖@���i�O�ς�����邽�߂ɒǉ��j
	vec3 axis[2]; // �����`�̎�
	float scale[2];        // �������̑傫��
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