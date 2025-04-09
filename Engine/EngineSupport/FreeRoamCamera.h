/**
* @file FreeRoamCamera.h
*/
#ifndef FREEROAMCAMERA_H_INCLUDED
#define FREEROAMCAMERA_H_INCLUDED
#include "../../Utility/MyGameFunction.h"
#include "../../Math/MyMath.h"
#include "../../Engine/Engine.h"
#include "../../Engine/Input/InputManager.h"


/**
* ワールド内を自由に飛び回るカメラ
*/
class FreeRoamCamera
{
public:
	vec3 position = vec3(0);
	float yaw = 0, pitch = 0;
	vec3 front = vec3(0);

	mat4 transMat = mat4(1);

	float speed = 10;	// 移動速度

	// 垂直視野角(弧度法)
	float fovY = 1.047f;

	// 視野角による拡大率の逆数
	// x: 視野角による水平拡大率
	// y: 視野角による垂直拡大率
	vec2 fovScale = vec2(0);

	float near = 0.35f;   // 近平面
	float far = 1000.0f;  // 遠平面

	/*深度値の計算結果が-1～+1になるようなパラメータA, B*/
	float A = 0.01f;      // 遠近法パラメータA
	float B = 0.99f;      // 遠近法パラメータB
	
private:
	mat4 matView = mat4(1);	// ビュー行列
	mat4 matProj = mat4(1);	// ビュープロジェクション行列

public:
	FreeRoamCamera(const vec3 position, const vec3 rotation) 
		:position(position), yaw(rotation.y), pitch(rotation.x)
	{
		// カメラのpitchが上限や下限に達しないように制限する
		pitch = std::clamp(pitch, -1.55f, 1.55f);

		front = {
			std::cos(yaw) * std::cos(pitch),
			std::sin(pitch),
			std::sin(yaw) * std::cos(pitch)
		};
		front = normalize(front);

		transMat = 
			TransformMatrix(position, vec3(pitch, yaw, 0), vec3(1));
	}
	virtual ~FreeRoamCamera() = default;

	// ビュー行列を取得する
	inline const mat4& GetViewMatrix() const {
		return matView;
	}

	// ビュープロジェクション行列を取得する
	inline const mat4& GetProjectionMatrix() const {
		return matProj;
	}

	void Update(float deltaSeconds, const Engine& engine)
	{
		// 移動処理
		if (engine.FocusWindow()) {
			Mouse* mouse = InputManager::GetMouse();

			// 右ボタンを押している間だけ処理
			if (mouse->GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)) {

				// マウスの移動量を取得
				vec2 inputValue = mouse->GetDeltaMousePos();
				yaw -= inputValue.x;
				pitch -= inputValue.y;

				// カメラのpitchが上限や下限に達しないように制限する
				pitch = std::clamp(pitch, -1.55f, 1.55f);

				front = {
					-std::cos(pitch) * std::sin(yaw),
					 std::sin(pitch),
					-std::cos(pitch) * std::cos(yaw)
				};
				front = normalize(front);


				Keyboard* keyBoard = InputManager::GetKeyboard();
				// 視野角を変更
				if (keyBoard->GetKey(GLFW_KEY_Z)) {
					fovY += deltaSeconds;
					fovY = std::min(fovY, 2.5f);
				}
				if (keyBoard->GetKey(GLFW_KEY_C)) {
					fovY -= deltaSeconds;
					fovY = std::max(fovY, 0.1f);
				}
				if (keyBoard->GetKey(GLFW_KEY_X)) {
					fovY = 1.047f;
				}
				// 速度を変更
				if (keyBoard->GetKey(GLFW_KEY_1)) {
					speed += 15 * deltaSeconds;
				}
				if (keyBoard->GetKey(GLFW_KEY_3)) {
					speed -= 15 * deltaSeconds;
					speed = std::max(speed, 0.5f);
				}

				vec3 moveVector;
				// 前進
				if (keyBoard->GetKey(GLFW_KEY_W)) {
					moveVector += front;
				}
				// 後退
				if (keyBoard->GetKey(GLFW_KEY_S)) {
					moveVector -= front;
				}
				// 左に移動
				if (keyBoard->GetKey(GLFW_KEY_A)) {
					moveVector -= normalize(cross(front, vec3(0, 1, 0)));
				}
				// 右に移動
				if (keyBoard->GetKey(GLFW_KEY_D)) {
					moveVector += normalize(cross(front, vec3(0, 1, 0)));
				}

				position += moveVector * speed * deltaSeconds;

				// 上に移動
				if (keyBoard->GetKey(GLFW_KEY_E)) {
					position.y += 5 * deltaSeconds;
				}
				// 下に移動
				if (keyBoard->GetKey(GLFW_KEY_Q)) {
					position.y -= 5 * deltaSeconds;
				}

				transMat = TransformMatrix(position, vec3(pitch, yaw, 0), vec3(1));
			}
		}
		
		// 遠近法パラメータA, Bを計算
		// （深度値の計算結果が-1～+1になるようなパラメータA, B）
		/* パースペクティブ行列の逆の計算を行い、ビュー座標系に変換される */
		A = -2 * far * near / (far - near);
		B = (far + near) / (far - near);

		// 視野角による拡大率を計算
		fovScale.y =	// 垂直
			1 / tan(fovY / 2);
		fovScale.x =	//水平
			fovScale.y * (1 / engine.GetAspectRatio());

		// ビュー行列を作成
		const vec3 eye = position;
		const vec3 target = position * front;
		matView = LookAt(eye, target, vec3(0, 1, 0));

		// ビュープロジェクション行列を作成
		matProj = Perspective(fovY, engine.GetAspectRatio(), near, far);
	}
};

#endif // FREEROAMCAMERA_H_INCLUDED