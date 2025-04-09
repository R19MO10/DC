/**
* @file ColliderDetection.cpp
*/
#include "ColliderDetection.h"

#include "../GameObject.h"
#include "../../Component/Collider.h"
#include "../../Component/AabbCollider.h"
#include "../../Component/BoxCollider.h"
#include "../../Component/SphereCollider.h"
#include "../../Component/Rigidbody.h"


void WorldCollider::AddPosition(const vec3& v)
{
	origin->GetOwner()->position += v;
	world->AddPosition(v);
}

namespace {
	/**
	* 貫通ベクトルをゲームオブジェクトに反映する(コライダーが重ならないように座標を調整)
	*/
	void ApplyPenetration(WorldColliderList* worldColliders,
		GameObject* gameObject, const vec3& penetration)
	{
		// リジッドボディがあれば接地判定を行う
		if (auto rigid = gameObject->GetRigidbody()) {
			// 衝突ベクトルが垂直に近い場合に、床に触れたとみなす
			static const float cosGround = cos(degrees_to_radians(30)); // 床とみなす角度

			// 衝突角度が90度より大きい場合は
			// 明らかに壁(天井)なので除外する
			if (penetration.y > 0) {

				// 対象が単位垂直ベクトルであることを利用して、
				// 内積による角度の比較を単純化
				const float d = length(penetration);

				if (penetration.y >= d * cosGround) {
					rigid->isGrounded = true; // 接地した
				}
			} // if penetration.y
		}

		// ゲームオブジェクトを移動
		gameObject->position += penetration;

		// 全てのワールドコライダーを移動
		for (auto& e : *worldColliders) {
			e.world->AddPosition(penetration);
		}
	}

	/**
	* コライダー型に対応する衝突判定関数を呼び出す
	*/
	template<typename ColA, typename ColB>
	bool Func(const ColliderPtr& a, const ColliderPtr& b, ContactPoint& cp)
	{
		return Intersect(static_cast<ColA&>(*a).GetShape(),
			static_cast<ColB&>(*b).GetShape(), cp);
	}

	template<typename ColA, typename ColB>
	bool NotFunc(const ColliderPtr& a, const ColliderPtr& b, ContactPoint& cp) {
		return false;
	}

	/**
	* コライダー単位の衝突判定
	*
	* @param a 判定対象のワールドコライダー配列その1
	* @param b 判定対象のワールドコライダー配列その2
	*/
	void HandleWorldColliderCollision(
		WorldColliderList* a, WorldColliderList* b)
	{
		// 関数ポインタ型を定義
		using FuncType = bool(*)(const ColliderPtr&, const ColliderPtr&, ContactPoint&);

		// 組み合わせに対応する交差判定関数を選ぶための配列
		static const FuncType funcList[3][3] = {
			{	// AABB
				Func<AabbCollider, AabbCollider>,		Func<AabbCollider, SphereCollider>,		Func<AabbCollider, BoxCollider>
			},
			{	// 球体
				Func<SphereCollider, AabbCollider>,		Func<SphereCollider, SphereCollider>,	Func<SphereCollider, BoxCollider>
			},
			{	// Box
				Func<BoxCollider, AabbCollider>,		Func<BoxCollider, SphereCollider>,		Func<BoxCollider, BoxCollider>,
			},
		};

		// コライダー単位の衝突判定
		for (auto& colA : *a) {
			for (auto& colB : *b) {
				// スタティックコライダー同士は衝突しない
				if (colA.origin->isStatic && colB.origin->isStatic) {
					continue;
				}

				bool makeDetection = true;	// 判定を行うか
				// 衝突判定を行わない組み合わせではないか調べる
				for (const auto& typePair : NonCollidablePairs) {
					if (colA.origin->type == typePair.first) {
						if (colB.origin->type == typePair.second) {
							makeDetection = false;
							break;
						}
					}
					if (colA.origin->type == typePair.second) {
						if (colB.origin->type == typePair.first) {
							makeDetection = false;
							break;
						}
					}
				}

				// 衝突判定を行うか
				if (!makeDetection) continue;

				const int typeA = static_cast<int>(colA.origin->GetType());
				const int typeB = static_cast<int>(colB.origin->GetType());
				ContactPoint contactPoint;

				// 衝突判定を行う
				if (funcList[typeA][typeB](colA.world, colB.world, contactPoint)) {
					//それぞれのコライダーを所有するゲームオブジェクトを取得
					GameObject* gObjA = colA.origin->GetOwner();
					GameObject* gObjB = colB.origin->GetOwner();

					// コライダーが重ならないように座標を調整
					if (!colA.origin->isTrigger && !colB.origin->isTrigger) {
						if (colA.origin->isStatic || 
							colA.origin->priority > colB.origin->priority) {
							// Aは動かないのでBを移動させる
							ApplyPenetration(
								b, gObjB, contactPoint.penetration);
						}
						else if (colB.origin->isStatic || 
								 colA.origin->priority < colB.origin->priority) {
							// Bは動かないのでAを移動させる
							ApplyPenetration(
								a, gObjA, -contactPoint.penetration);
						}
						else {
							// AとBを均等に移動させる
							ApplyPenetration(
								b, gObjB, contactPoint.penetration * 0.5f);
							ApplyPenetration(
								a, gObjA, contactPoint.penetration * -0.5f);
						}
					}

					// イベント関数の呼び出し
					gObjA->OnCollisionHit(colA.origin, colB.origin, contactPoint);
					gObjB->OnCollisionHit(colB.origin, colA.origin, contactPoint);

					// イベントの結果、どちらかのゲームオブジェクトが破棄されたらループ終了
					if (gObjA->IsDestroyed() || gObjB->IsDestroyed()) {
						return; // 関数を終了
					}
				}
			} // for colB
		} // for colA
	}
}

/**
* コライダーの衝突判定
*
* @param colliders			判定するコライダーの配列
*/
void ColliderDetection(
	std::vector<WorldColliderList>* colliders)
{
	//判定するコライダー(判定するゲームオブジェクトの数)が２つ未満なら判定しない
	if (colliders->size() < 2) {
		return;
	}

	//判定するコライダー(判定するゲームオブジェクトの数)２つ以上

	// ゲームオブジェクト単位の衝突判定
	for (auto a = colliders->begin(); a != colliders->end() - 1; ++a) {
		if (a->empty()) {
			// 判定するコライダーがなかったら飛ばす
			continue;
		}
		//コライダーを所有するゲームオブジェクトを取得
		const GameObject* goA = a->at(0).origin->GetOwner();

		if (goA->IsDestroyed()) {
			continue; // 削除済みなので飛ばす
		}

		for (auto b = a + 1; b != colliders->end(); ++b) {
			if (b->empty()) {
				// 判定するコライダーがなかったら飛ばす
				continue;
			}
			//コライダーを所有するゲームオブジェクトを取得
			const GameObject* goB = b->at(0).origin->GetOwner();

			if (goB->IsDestroyed()) {
				continue; // 削除済みなので飛ばす
			}

			// コライダー単位の衝突判定
			HandleWorldColliderCollision(&*a, &*b);  /*「&*」イテレータからポインタに変換*/

		} // for b
	} // for a
}