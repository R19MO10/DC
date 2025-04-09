/**
* @file ColliderDetection.h
*/
#ifndef COLLIDER_DETECTION_H_INCLUDED
#define COLLIDER_DETECTION_H_INCLUDED
#include "../../Math/MyMath.h"
#include "../Collision/Collision.h"
#include <memory>
#include <vector>

// 先行宣言
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;


// ワールド座標系のコライダーを表す構造体
struct WorldCollider
{
	ColliderPtr origin;	// オリジナルコライダー
	ColliderPtr world;	// ワールド座標適応後のコライダー

	// 座標を変更する
	void AddPosition(const vec3& v);
};
using WorldColliderList = std::vector<WorldCollider>;

/**
* コライダーの衝突判定
* 
* @param colliders		判定するコライダーの配列
*/
void ColliderDetection(
	std::vector<WorldColliderList>* colliders);

#endif // COLLISION_DETECTION_H_INCLUDED