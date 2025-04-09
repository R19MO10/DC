/**
* @file MyGameFunction.cpp
*/
#include "MyGameFunction.h"

#include "../Engine/GameObject.h"
#include "../Engine/Component.h"

/**
* ターゲット座標が自身との相対的な前後の位置を判定
*
* @param self			自身のゲームオブジェクト
* @param targetPosition	ターゲットの座標
*
* @return 「1」前, 「-1」後
*/
int RelativeFrontBackDirection(
	const GameObject* self, const vec3 targetPosition)
{
	// 自身の右正面ベクトル
	vec3 ownerForward =
		normalize(vec3(sin(self->rotation.y), 0.0f, cos(self->rotation.y))) - halfPI;

	// 自身から相手までの方向ベクトル
	vec3 toOtherDirection = normalize(targetPosition - self->position);
	toOtherDirection.y = 0.0f;	// 方向ベクトルをXZ平面にする

	// 自身の右正面ベクトルと相手への方向ベクトルの外積を計算
	vec3 crossDirec = cross(ownerForward, toOtherDirection);

	return (crossDirec.y > 0 ? 1 : -1);	// 1：前,  -1：後
}
int RelativeFrontBackDirection(
	const GameObject* self, const GameObject* targetObj)
{
	return RelativeFrontBackDirection(self, targetObj->position);
}


/**
* ターゲット座標が自身との相対的な左右の位置を判定
*
* @param self			自身のゲームオブジェクト
* @param targetPosition	ターゲットの座標
*
* @return 「1」左, 「-1」右
*/
int RelativeLeftRightDirection(
	const GameObject* self, const vec3 targetPosition)
{
	// 自身の正面ベクトル
	vec3 ownerForward =
		normalize(vec3(sin(self->rotation.y), 0.0f, cos(self->rotation.y)));

	// 自身から相手までの方向ベクトル
	vec3 toOtherDirection = normalize(targetPosition - self->position);
	toOtherDirection.y = 0.0f;	// 方向ベクトルをXZ平面にする

	// 自身の右正面ベクトルと相手への方向ベクトルの外積を計算
	vec3 crossDirec = cross(ownerForward, toOtherDirection);

	return (crossDirec.y > 0 ? 1 : -1);	// 1：左,  -1：右
}
int RelativeLeftRightDirection(
	const GameObject* self, const GameObject* targetObj)
{
	return RelativeLeftRightDirection(self, targetObj->position);
}

/**
* ゲームオブジェクトが有効か調べる
*/
bool isValid(const GameObject* gameObject) 
{
	return (gameObject && !gameObject->IsDestroyed());
}
bool isValid(const GameObjectPtr& gameObject)
{
	return (isValid(gameObject.get()));
}

/**
* コンポーネントが有効か調べる
*/
bool isValid(const Component* component) 
{
	return (component && !component->IsDestroyed());
}
bool isValid(const ComponentPtr& component)
{
	return (isValid(component.get()));
}