/**
* @file Camera.h
*/
#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED
#include "../Engine/Component.h"
#include "../Math/MyMath.h"

/**
* カメラコンポーネント
*/
class Camera : public Component
{
public:
	// 垂直視野角(弧度法)
	float fovY = 1.047f;

	// 視野角による拡大率の逆数
	// x: 視野角による水平拡大率
	// y: 視野角による垂直拡大率
	vec2 fovScale = vec2(0);;

	float near = 0.35f;   // 近平面
	float far = 1000.0f;  // 遠平面

	/*深度値の計算結果が-1～+1になるようなパラメータA, B*/
	float A = 0.01f;      // 遠近法パラメータA
	float B = 0.99f;      // 遠近法パラメータB

private:
	float shake_elapsedTime_s = 0;	// シェイク開始からの経過時間
	float maxShakePower = 0;		// シェイクの強さ
	float currentShakePower = 0;	// 現在のシェイクの強さ

public:
	bool isShake = false;	// カメラシェイクの実行の有無
	float shakeSpeed = 150;	// シェイク速度

private:
	mat4 matView = mat4(1);	// ビュー行列
	mat4 matProj = mat4(1);	// ビュープロジェクション行列

public:
	Camera() 
		:Component("Camera")
	{};
	virtual ~Camera() = default;

public:
	// ビュー行列を取得する
	inline const mat4& GetViewMatrix() const {
		return matView;
	}

	// ビュープロジェクション行列を取得する
	inline const mat4& GetProjectionMatrix() const {
		return matProj;
	}

public:
	// カメラ振動を実行
	inline void CameraShake(float power) {
		if (power <= 0) {
			// 振動パワーが0以下なら処理しない
			return;
		}

		isShake = true;

		power *= 0.001f;	//適切な振動強度まで調整

		// 現在行われている振動よりも強かったら適応する
		if (power > currentShakePower) {
			currentShakePower = maxShakePower = power;
		}
	}

public:
	virtual void EndUpdate(
		float deltaSeconds, mat4& ownerWorldTransMat) override;
	virtual void DrawImGui() override;
};
using CameraPtr = std::shared_ptr<Camera>;

#endif // CAMERA_H_INCLUDED