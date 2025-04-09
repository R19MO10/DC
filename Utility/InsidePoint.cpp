/**
* InsidePoint.cpp
*/
#include "InsidePoint.h"

#include "../Engine/GameObject.h"
#include "../Component/AabbCollider.h"
#include "../Component/SphereCollider.h"
#include "../Component/BoxCollider.h"
#include <algorithm>


namespace {
	/**
	* 光線とコライダーの接触判定を実行
	*/
	bool ExecutInsidePoint(const vec3& point, const ColliderPtr& collider)
	{
		// コライダーをワールド座標系に変換
		const auto worldCollider =
			collider->GetTransformedCollider(collider->GetOwner()->GetWorldTransformMatrix());

		// 点との接触判定
		switch (collider->GetType()) {
		case Collider::Type::AABB:
			return Inside(static_cast<AabbCollider&>(*worldCollider).aabb, point);
			break;

		case Collider::Type::Sphere:
			return Inside(static_cast<SphereCollider&>(*worldCollider).sphere, point);
			break;

		case Collider::Type::Box:
			return Inside(static_cast<BoxCollider&>(*worldCollider).box, point);
			break;
		}

		return false;
	}
}


/**
* AABBと点の接触判定
*
* @param[in]  aabb		判定対象の球体
* @param[in]  point		判定対象の点
*
* @retval true  接触している
* @retval false 接触していない
*/
bool Inside(const AABB& aabb, const vec3 point)
{
	// 最近接点までの距離が0よりも長ければ、接触していない
	const vec3 p = Calc::ClosestPoint(aabb, point);
	const vec3 v = point - p;
	const float d2 = dot(v, v);
	if (d2 > 0) {
		return false;
	}
	return true;
}

/**
* 球体と点の接触判定
*
* @param[in]  sphere	判定対象の球体
* @param[in]  point		判定対象の点
*
* @retval true  接触している
* @retval false 接触していない
*/
bool Inside(const Sphere& sphere, const vec3 point)
{
	const vec3 v = sphere.position - point;
	const float d2 = dot(v, v); // vの長さの2乗
	// d2が球体の半径より長い場合は接触していない
	if (d2 > sphere.radius * sphere.radius) { // 平方根を避けるため、2乗同士で比較する
		return false;
	}
	return true;
}

/**
* OBBと点の接触判定
*
* @param[in]  box		判定対象のOBB
* @param[in]  point		判定対象の点
*
* @retval true  接触している
* @retval false 接触していない
*/
bool Inside(const Box& box, const vec3 point)
{
	// 最近接点までの距離が0よりも長ければ、接触していない
	const vec3 p = Calc::ClosestPoint(box, point);
	const vec3 v = point - p;
	const float d2 = dot(v, v);
	if (d2 > 0) {
		return false;
	}
	return true;
}


/**
*  点とコライダーの接触判定
*/
bool InsidePoint(
	const vec3& point, const ColliderPtr& collider, const InsidePointPredicate& pred)
{
	// 接触判定の対象でなければ飛ばす
	/* Raycastと異なり距離の計算は行わないため接触判定前に行う事ができる */
	if (!pred(collider)) {
		return false;
	}

	// 接触判定を実行
	if (!ExecutInsidePoint(point, collider)) {
		return false;
	}

	// 接触している
	return true;
}

/**
* 点とコライダーリストの接触判定
*/
ColliderList InsidePoint(
	const vec3& point, const ColliderList& colliders, const InsidePointPredicate& pred)
{
	ColliderList hitColliders;

	for (const auto& col : colliders) {

		// 接触判定の対象でなければ飛ばす
		/* Raycastと異なり距離の計算は行わないため接触判定前に行う事ができる */
		if (!pred(col)) {
			continue;
		}

		// 接触判定を実行
		if (!ExecutInsidePoint(point, col)) {
			continue;
		}

		// 接触しているため接触コライダー配列に追加する
		hitColliders.push_back(col);
	}

	return hitColliders;
}

/**
*  点とゲームオブジェクトコライダーの接触判定
*/
ColliderList InsidePoint(
	const vec3& point, const GameObjectList& gameObjects,  const InsidePointPredicate& pred)
{	
	ColliderList hitColliders;

	for (const auto& gObj : gameObjects) {
		for (const auto& col : gObj->GetColliders()) {
			// 接触判定の対象でなければ飛ばす
			/* Raycastと異なり距離の計算は行わないため接触判定前に行う事ができる */
			if (!pred(col)) {
				continue;
			}

			// 接触判定を実行
			if (!ExecutInsidePoint(point, col)) {
				continue;
			}

			// 接触しているため接触コライダー配列に追加する
			hitColliders.push_back(col);
		}
	}

	return hitColliders;
}
