/**
* @file Billboard.h
*/
#ifndef BILLBOARD_H_INCLUDED
#define BILLBOARD_H_INCLUDED
#include "../Engine/Component.h"
#include "../Engine/Engine.h"
#include "../Engine/GameObject.h"

/**
* ビルボード
*/
class Billboard : public Component
{
public:
	Billboard(const char* name = "Billboard")
		:Component(name)
	{};
	virtual ~Billboard() = default;

	virtual void EndUpdate(
		float deltaSeconds, mat4& ownerWorldTransMat) override 
	{
		// 常にカメラの方を向くようにメッセージオブジェクトの向きを調整
		if (GameObject* camera = GetOwner()->GetEngine()->GetMainCameraObject()) {
			// 拡大率を抽出
			const vec3 scale = ExtractScale(ownerWorldTransMat);

			// ビルボード反映後の座標変換行列を作成
			ownerWorldTransMat =
				TransformMatrix(vec3(ownerWorldTransMat[3]), camera->rotation, scale);
		}
	}
};
using BillboardPtr = std::shared_ptr<Billboard>;

#endif // BILLBOARD_H_INCLUDED