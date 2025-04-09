/**
* @file MyGameFunction.h
*/
#ifndef MYGAMEFUCTION_INCLUDE_H
#define MYGAMEFUCTION_INCLUDE_H
#include "../Math/MyMath.h"
#include "Random.h"
#include <memory>

// 先行宣言
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
class Component;
using ComponentPtr = std::shared_ptr<Component>;


// RGB値を0～255から0～1の範囲に変換する
inline float UnitRangeRGB(const float& color255) {
	return color255 / 255.0f;
}
inline vec3 UnitRangeRGB(const vec3& color255) {
	return vec3(color255 / 255.0f);
}
inline vec4 UnitRangeRGB(const vec4& color255) {
	return vec4(color255 / 255.0f);
}

// ターゲットの方向を向くための回転を計算する
inline vec3 GetRotationToLookAt(const vec3& position, const vec3& target) {
	// 自身からターゲットへの方向ベクトルを計算
	const vec3 direction = normalize(target - position);

	// 方向ベクトルからオイラー角を計算
	float pitch = std::asin(direction.y);
	// Y軸を計算するためにアークタンジェントを使用
	float yaw = std::atan2(-direction.x, -direction.z); 

	return { pitch, yaw, 0.0f }; // ロールは0に設定（この方法では計算されない）
}

// 2つのベクトルAとBから角度(ラジアン)を計算する
inline float AngleBetweenVectors(const vec3& a, const vec3& b) 
{
	// ベクトルAとベクトルBの内積を計算
	float dotProduct = dot(a, b);

	// ベクトルAとベクトルBの大きさを計算
	float magnitudeA =length(a);
	float magnitudeB =length(b);

	// アークコサインを使用してラジアンで角度を計算
	float angle = std::acos(dotProduct / (magnitudeA * magnitudeB));

	return angle;
}

// 正規化された2つのベクトルAとBから角度(ラジアン)を計算する
inline float AngleBetweenNormalizedVectors(const vec3& na, const vec3& nb)
{
	// ベクトルAとベクトルBの内積を計算（これは正規化されたベクトルなので、長さは1）
	float dotProduct = dot(na, nb);

	// アークコサインを使用してラジアンで角度を計算
	float angle = std::acos(dotProduct);

	return angle;
}

/**
* ターゲット座標が自身との相対的な前後の位置を判定
* 
* @param self			自身のゲームオブジェクト
* @param targetPosition	ターゲットの座標
* 
* @return 「1」前, 「-1」後
*/
int RelativeFrontBackDirection(
	const GameObject* self, const vec3 targetPosition);
int RelativeFrontBackDirection(
	const GameObject* self, const GameObject* targetObj);

/**
* ターゲット座標が自身との相対的な左右の位置を判定
*
* @param self			自身のゲームオブジェクト
* @param targetPosition	ターゲットの座標
*
* @return 「1」左, 「-1」右
*/
int RelativeLeftRightDirection(
	const GameObject* self, const vec3 targetPosition);
int RelativeLeftRightDirection(
	const GameObject* self, const GameObject* targetObj);

/**
* ゲームオブジェクトが有効か調べる
*/
bool isValid(const GameObject* gameObject);
bool isValid(const GameObjectPtr& gameObject);

/**
* コンポーネントが有効か調べる
*/
bool isValid(const Component* component);
bool isValid(const ComponentPtr& component);

#endif // MYGAMEFUCTION_INCLUDE_H