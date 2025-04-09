/**
* InsidePoint.h
*/
#ifndef INSIDEPOINT_H_INCLUDED
#define INSIDEPOINT_H_INCLUDED

#include "../Math/MyMath.h"
#include "../Engine/Collision/Collision.h"
#include "../Component/Collider.h"
#include <functional>

// 先行宣言
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
using GameObjectList = std::vector<GameObjectPtr>;


/**
* 接触判定の対象になるかどうかを調べる述語型
*
* @param collider 判定するコライダー
*
* @retval true  接触判定の対象になる
* @retval false 接触判定の対象にならない
*/
using InsidePointPredicate =
    std::function<bool(const ColliderPtr& collider)>;


// 各コライダーとの接触判定関数
bool Inside(const AABB& aabb,       const vec3 point);
bool Inside(const Sphere& sphere,   const vec3 point);
bool Inside(const Box& box,         const vec3 point);

/**
* 点とコライダーの接触判定
*
* @param point          点の座標
* @param collider       接触判定をするコライダーポインタ
* @param pred           接触判定を行うコライダーを選別する述語
* 
* @retval true  コライダーと接触した
* @retval false どのコライダーとも接触しなかった
*/
bool InsidePoint(
    const vec3& point, const ColliderPtr& collider, const InsidePointPredicate& pred);

/**
* 点とコライダーリストの接触判定
*
* @param point          点の座標
* @param colliders      接触判定をするコライダーポインタ
* @param pred           接触判定を行うコライダーを選別する述語
*
* @return 点と接触しているコライダーポインタ配列
*/
ColliderList InsidePoint(
    const vec3& point, const ColliderList& colliders, const InsidePointPredicate& pred);


/**
*  点とゲームオブジェクトコライダーの接触判定
*
* @param point          点の座標
* @param gameObjects    接触判定をするゲームオブジェクト配列
* @param pred           交差判定を行うコライダーを選別する述語
*
* @return 点と接触しているコライダーポインタ配列
*/
ColliderList InsidePoint(
    const vec3& point, const GameObjectList& gameObjects, const InsidePointPredicate& pred);

#endif // INSIDEPOINT_H_INCLUDED