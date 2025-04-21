/**
* @file CharacterComponent.h
*/
#ifndef CHARACTER_COMPONENT_H_INCLUDED
#define CHARACTER_COMPONENT_H_INCLUDED
#include "../Engine/Component.h"

/**
* キャラクターコンポーネントベース
*/
class CharacterComponent : public Component
{
public:
	float lastDamage_elapsedTime_s = 0;	//最後のダメージからの経過時間

public:
	CharacterComponent(const char* name = "CharacterComponent")
		: Component(name)
	{}
	virtual ~CharacterComponent() = default;

public:
	/**
	* ダメージを受ける
	*
	* @param damage			ダメージ量
	* @param damageObject	自身にダメージを与えたゲームオブジェクトポインタ
	* @param contactPoint	衝突情報を保持する構造体
	*/
	virtual void AnyDamage(
		float damage, GameObject* damageObject, const ContactPoint& contactPoint) = 0;;

public:
	virtual void Update(float deltaSeconds) override {
		lastDamage_elapsedTime_s += deltaSeconds;
	}
};
using CharacterComponentPtr = std::shared_ptr<CharacterComponent>;

#endif // CHARACTER_COMPONENT_H_INCLUDED