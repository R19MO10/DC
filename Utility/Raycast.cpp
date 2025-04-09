/**
* @file Raycast.cpp
*/
#include "Raycast.h"

#include "../Engine/Input/InputManager.h"
#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"
#include "../Component/AabbCollider.h"
#include "../Component/SphereCollider.h"
#include "../Component/BoxCollider.h"

namespace {
	/**
	* スラブ(ある軸に垂直な2平面に囲まれた範囲)と光線の交差判定
	*
	* @param[in]  min       スラブの開始距離
	* @param[in]  max       スラブの終了距離
	* @param[in]  start     光線の発射点
	* @param[in]  direction 光線の向き
	* @param[out] tmin      AABBと光線の交差開始距離
	* @param[out] tmax      AABBと光線の交差終了距離
	*
	* @retval true  交差している
	* @retval false 交差していない
	*/
	bool IntersectSlab(float min, float max, float start, float direction, float& tmin, float& tmax)
	{
		// 光線がスラブと平行な場合
		// 発射点がスラブ内にあれば交差している、外にあれば交差していない
		if (abs(direction) < 0.0001f) {
			return (start >= min) && (start <= max);
		}

		// 光線とスラブが交差する開始時刻と終了時刻を求める
		float t0 = (min - start) / direction;
		float t1 = (max - start) / direction;

		// 時刻の早い側を開始時刻とする
		//光線がマイナス方向に向かっている場合に開始点・終了点が逆になるのを防ぐ
		if (t0 > t1) {
			std::swap(t0, t1);
		}

		// 共通の交差範囲を求める
		// 以前の開始時刻と今回の開始時刻を比較し、遅いほうを選択
		if (t0 > tmin) {
			tmin = t0;
		}

		// 共通の公差範囲を求める
		// 以前の終了時刻と今回の終了時刻を比較し、早いほうを選択
		if (t1 < tmax) {
			tmax = t1;
		}

		// 「開始時刻 <= 終了時刻」の場合は交差している
		return tmin <= tmax;
	}

	/**
	* スラブ(ある軸に垂直な2平面に囲まれた範囲)と光線の交差判定
	*
	* @param[in]  axis      スラブの軸ベクトル
	* @param[in]  scale     スラブの幅
	* @param[in]  start     光線の発射点(スラブの中心を原点とする)
	* @param[in]  direction 光線の向き
	* @param[out] tmin      AABBと光線の交差開始距離
	* @param[out] tmax      AABBと光線の交差終了距離
	*
	* @retval true  交差している
	* @retval false 交差していない
	*/
	bool IntersectSlab(const vec3& axis, float scale, const vec3& start, const vec3& direction,
		float& tmin, float& tmax)
	{
		// 向きベクトルと発射点について、軸ベクトル方向の成分を求める
		const float e = dot(axis, direction);	// WHAT?
		const float f = dot(axis, start);

		// 光線がスラブと平行な場合
		// 発射点がスラブ内にあれば交差している、外にあれば交差していない
		if (abs(e) < 0.0001f) {
			return (f >= -scale) && (f <= scale);
		}

		// 光線とスラブが交わる範囲の開始時刻と終了時刻を求める
		float t0 = (-scale - f) / e;
		float t1 = (scale - f) / e;

		// 時刻の早い側を開始時刻とする
		if (t0 > t1) {
			const float tmp = t0;
			t0 = t1;
			t1 = tmp;
		}
		
		// 以前の開始時刻と今回の開始時刻を比較し、遅いほうを選択
		if (t0 > tmin) {
			tmin = t0;
		}
		
		// 以前の終了時刻と今回の終了時刻を比較し、早いほうを選択
		if (t1 < tmax) {
			tmax = t1;
		}
		
		// 「開始時刻 <= 終了時刻」の場合は交差している
		return tmin <= tmax;
	}


	/**
	* 光線とコライダーの交差判定を実行
	*/
	bool ExecutRaycast(const Ray& ray, const ColliderPtr& collider, float& distance)
	{
		// コライダーをワールド座標系に変換
		const auto worldCollider =
			collider->GetTransformedCollider(collider->GetOwner()->GetWorldTransformMatrix());

		// 光線との交差判定
		float d;
		bool hit = false;
		switch (collider->GetType()) {
		case Collider::Type::AABB:
			hit = Intersect(static_cast<AabbCollider&>(*worldCollider).aabb, ray, d);
			break;

		case Collider::Type::Sphere:
			hit = Intersect(static_cast<SphereCollider&>(*worldCollider).sphere, ray, d);
			break;

		case Collider::Type::Box:
			hit = Intersect(static_cast<BoxCollider&>(*worldCollider).box, ray, d);
			break;
		}

		distance = d;
		return hit;
	}
}

/**
* AABBと光線の交差判定
*
* @param[in]  aabb     判定対象のAABB
* @param[in]  ray      判定対象の光線
* @param[out] distance 光線がAABBと最初に交差する距離
*
* @retval true  交差している
* @retval false 交差していない
*/
bool Intersect(const AABB& aabb, const Ray& ray, float& distance)
{
	// 共通の交差範囲
	float tmin = 0;
	float tmax = FLT_MAX;

	// スラブとの交差判定
	for (int i = 0; i < 3; ++i) {
		if (!IntersectSlab(aabb.min[i], aabb.max[i],
			ray.start[i], ray.direction[i], tmin, tmax)) {
			return false; // 交差していない
		}
	}

	// 交点までの距離を設定
	distance = tmin;
	return true; // 交差している
}

/**
* 球体と光線の交差判定
*
* @param[in]  sphere   判定対象の球体
* @param[in]  ray      判定対象の光線
* @param[out] distance 光線が球体と最初に交差する距離
*
* @retval true  交差している
* @retval false 交差していない
*/
bool Intersect(const Sphere& sphere, const Ray& ray, float& distance)
{
	const vec3 m = ray.start - sphere.position;
	const float b = dot(m, ray.direction);
	const float c = dot(m, m) - sphere.radius * sphere.radius;

	// 光線の始点が球体の外にあり(c > 0)、光線が球体から離れていく方向に
	// 発射された(b > 0)場合、球体と光線は交差しない
	if (c > 0 && b > 0) {
		return false;
	}

	// 判別式が負の場合は交差しない
	const float disc = b * b - c; // 判別式
	if (disc < 0) {
		return false;
	}

	// 最初に交差する位置を計算
	distance = -b - sqrt(disc);

	// 負の位置は始点より手前を指し、光線が球体内から発射されたことを意味する
	// この場合、始点を「最初に交差する位置」とする
	if (distance < 0) {
		distance = 0;
	}

	return true;
}

/**
* OBBと光線の交差判定
*
* @param[in]  box      判定対象のOBB
* @param[in]  ray      判定対象の光線
* @param[out] distance 光線がOBBと最初に交差する距離
* 
* @retval true  交差している
* @retval false 交差していない
*/
bool Intersect(const Box& box, const Ray& ray, float& distance)
{
	// スラブ中心を原点とした場合の光線の発射点を計算
	const vec3 start = ray.start - box.position;

	// スラブとの交差判定
	float tmin = 0;
	float tmax = FLT_MAX;
	for (int i = 0; i < 3; ++i) {
		if (!IntersectSlab(box.axis[i], box.scale[i], start, ray.direction,
			tmin, tmax)) {
			return false; // 交差していない
		}
	}

	// 交点までの距離を設定
	distance = tmin;
	return true; // 交差している
}


/**
* マウス座標から発射される光線を取得する
*/
Ray GetRayFromMousePosition(GameObject* camera)
{
	// マウス座標(カメラ座標系(Z=-1))を取得
	vec2 mousePos = InputManager::GetMouse()->GetMousePosition();

	// NDC座標系に変換を作成
	vec3 nearPos(mousePos, -1);
	vec3 farPos(mousePos, 1);

	const auto cComp = camera->GetComponent<Camera>();

	// NDC座標系からクリップ座標系に変換
	nearPos *= cComp->near;
	farPos *= cComp->far;
	nearPos.z = (nearPos.z - cComp->A) / cComp->B;
	farPos.z = (farPos.z - cComp->A) / cComp->B;

	// クリップ座標系からビュー座標系に変換
	const float invFovScale = 1.0f / cComp->fovScale.y;	/*計算の除算を減らすために逆数にする*/
	nearPos *= invFovScale;
	farPos *= invFovScale;

	// ビュー座標系からワールド座標系に変換
	const float cameraSinY = std::sin(camera->rotation.y);
	const float cameraCosY = std::cos(camera->rotation.y);

	//光線の始まり(nearPos)と終わり(farPos)を求める
	nearPos = {
		nearPos.x * cameraCosY - cComp->near * cameraSinY,
		nearPos.y,
		nearPos.x * -cameraSinY - cComp->near * cameraCosY
	};
	nearPos += camera->position;

	farPos = {
		farPos.x * cameraCosY - cComp->far * cameraSinY,
		farPos.y,
		farPos.x * -cameraSinY - cComp->far * cameraCosY
	};
	farPos += camera->position;

	// 近平面の座標と遠平面の座標から光線の向きベクトルを求める
	/*ベクトルの正規化を行い最後にまとめて割ることで除算を減らし、処理を高速化している*/
	vec3 direction = normalize(farPos - nearPos);

	return Ray{ nearPos, direction };
}


/**
* 光線とコライダーポインタ
*/
bool Raycast(
	const Ray& ray, const ColliderPtr& collider,
	RayHitInfo& hitInfo, const RaycastPredicate& pred)
{
	// 交点の情報を初期化
	hitInfo.collider = nullptr;
	hitInfo.distance = FLT_MAX;

	float d;
	if (!ExecutRaycast(ray, collider, d)) {
		// 交差していない
		return false;
	}

	if (!pred(collider, d)) {
		// 交差判定の対象でない
		return false;
	}

	hitInfo.collider = collider;
	hitInfo.distance = d;
	hitInfo.point = ray.start + ray.direction * hitInfo.distance;

	return true;
}

/**
* 光線とコライダーリスト
*/
bool Raycast(
	const Ray& ray, const ColliderList& colliders,
	RayHitInfo& hitInfo, const RaycastPredicate& pred)
{
	// 交点の情報を初期化
	hitInfo.collider = nullptr;
	hitInfo.distance = FLT_MAX;

	for (const auto& col : colliders) {

		// 交差判定を実行
		float d;
		if (!ExecutRaycast(ray, col, d)) {
			// 交差していない
			continue;
		}

		// 交差判定の対象でなければ飛ばす
		if (!pred(col, d)) {
			continue;
		}

		// より発射点に近い交点を持つコライダーを選ぶ
		if (d < hitInfo.distance) {
			hitInfo.collider = col;
			hitInfo.distance = d;
		}

	} // for colliders

	// 交差するコライダーがあれればtrue、なければfalseを返す
	if (hitInfo.collider) {
		// 交点の座標を計算
		hitInfo.point = ray.start + ray.direction * hitInfo.distance;
		return true;
	}
	return false;
}

/**
* 光線とゲームオブジェクトコライダー
*/
bool Raycast(
	const Ray& ray, const GameObjectList& gameObjects, 
	RayHitInfo& hitInfo, const RaycastPredicate& pred)
{
	// 交点の情報を初期化
	hitInfo.collider = nullptr;
	hitInfo.distance = FLT_MAX;

	for (const auto& gObj : gameObjects) {	/*Unityはもっと賢いやり方で行っている*/
		for (const auto& col : gObj->GetColliders()) {
			// 交差判定を実行
			float d;
			if (!ExecutRaycast(ray, col, d)) {
				// 交差していない
				continue;
			}

			// 交差判定の対象でなければ飛ばす
			if (!pred(col, d)) {
				continue;
			}

			// より発射点に近い交点を持つコライダーを選ぶ
			if (d < hitInfo.distance) {
				hitInfo.collider = col;
				hitInfo.distance = d;
			}

		} // for colliders
	} // for gameObjects

	// 交差するコライダーがあれればtrue、なければfalseを返す
	if (hitInfo.collider) {
		// 交点の座標を計算
		hitInfo.point = ray.start + ray.direction * hitInfo.distance;
		return true;
	}
	return false;
}