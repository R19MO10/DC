/**
* @file Raycast.h
*/
#ifndef RAYCAST_H_INCLUDED
#define RAYCAST_H_INCLUDED
#include "../Math/MyMath.h"
#include "../Engine/Collision/Collision.h"
#include "../Component/Collider.h"
#include <functional>

// 先行宣言
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
using GameObjectList = std::vector<GameObjectPtr>;

/**
* 光線
*/
struct Ray
{
    vec3 start;     // 光線の始点
    vec3 direction; // 光線の向き
};

// 光線の交差判定結果
struct RayHitInfo
{
    ColliderPtr collider;     // 最初に光線と交差したコライダー
    vec3 point = vec3(0);     // 最初の交点の座標
    float distance = 0;       // 最初の交点までの距離
};

/**
* 交差判定の対象になるかどうかを調べる述語型
*
* @param collider 判定するコライダー
* @param distance 光線とコライダーの交点までの距離
*
* @retval true  交差判定の対象になる
* @retval false 交差判定の対象にならない
*/
using RaycastPredicate =
    std::function<bool(const ColliderPtr& collider, float distance)>;


// 各コライダーとの交差判定関数
bool Intersect(const AABB& a,   const Ray& b, float& distance);
bool Intersect(const Sphere& a, const Ray& b, float& distance);
bool Intersect(const Box& a,    const Ray& b, float& distance);


/**
* マウス座標から発射される光線を取得する(指定したカメラ)
*
* @param mainCmaera カメラオブジェクト
* 
* @return マウス座標から発射される光線
*/
Ray GetRayFromMousePosition(GameObject* camera);


/**
* 光線とコライダーの交差判定
*
* @param ray            光線
* @param collider       接触判定をするコライダーポインタ
* @param hitInfo        光線と最初に交差したコライダーの情報
* @param pred           交差判定を行うコライダーを選別する述語
*
* @retval true  コライダーと交差した
* @retval false どのコライダーとも交差しなかった
*/
bool Raycast(const Ray& ray, const ColliderPtr& collider, 
    RayHitInfo& hitInfo, const RaycastPredicate& pred);

/**
* 光線とコライダーリストの交差判定
*
* @param ray            光線
* @param colliders      接触判定をするコライダー配列
* @param hitInfo        光線と最初に交差したコライダーの情報
* @param pred           交差判定を行うコライダーを選別する述語
*
* @retval true  コライダーと交差した
* @retval false どのコライダーとも交差しなかった
*/
bool Raycast(const Ray& ray, const ColliderList& colliders,
    RayHitInfo& hitInfo, const RaycastPredicate& pred);

/**
* 光線とゲームオブジェクトコライダーの交差判定
*
* @param ray            光線
* @param gameObjects    接触判定をするゲームオブジェクト配列
* @param hitInfo        光線と最初に交差したコライダーの情報
* @param pred           交差判定を行うコライダーを選別する述語
*
* @retval true  コライダーと交差した
* @retval false どのコライダーとも交差しなかった
*/
bool Raycast(const Ray& ray, const GameObjectList& gameObjects, 
    RayHitInfo& hitInfo, const RaycastPredicate& pred);


#endif  // RAYCAST_H_INCLUDED