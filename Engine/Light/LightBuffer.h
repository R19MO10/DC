/**
* @file LightBuffer.h
*/
#ifndef LIGHTBUFFER_H_INCLUDE
#define LIGHTBUFFER_H_INCLUDE
#include <vector>
#include <memory>
#include <utility>
#include "../../UniformLocation.h"
#include "../../Math/MyMath.h"

// 先行宣言
class LightBuffer;
using LightBufferPtr = std::shared_ptr<LightBuffer>;
class Engine;
class MappedBufferObject;
using MappedBufferObjectPtr = std::shared_ptr<MappedBufferObject>;
class GameObject;
class Light;
using LightPtr = std::shared_ptr<Light>;

/**
* ゲーム内のライトコンポーネントを管理するクラス
*/
class LightBuffer
{
	friend Engine;
private:
	static constexpr size_t lightResizeCount = 100;	// 一度に増やすライト数

	std::vector<LightPtr> lights;	// ライトコンポーネント配列

	// SSBOライトデータ
	struct SSBOLightData
	{
		vec3 color = vec3(0);		// 色と明るさ
		float falloffAngle = 0;		// スポットライトの減衰開始角度(Spot)

		vec3 position = vec3(0);	// 位置
		float radius = 0;			// ライトが届く最大半径

		vec3 direction = vec3(0);   // ライトの向き(Spot)
		float coneAngle = 0;		// スポットライトが照らす角度(Spot)
	};

	MappedBufferObjectPtr ssboLight;	// ライト用SSBO

public:
	/**
	* ライトバッファを作成する
	*
	* @param bufferSize_byte 格納できる頂点データのサイズ(バイト数)
	*/
	static LightBufferPtr Create()
	{
		return std::make_shared<LightBuffer>();
	}

	// コンストラクタ・デストラクタ
	LightBuffer();
	~LightBuffer() = default;

	// コピーと代入を禁止
	LightBuffer(const LightBuffer&) = delete;
	LightBuffer& operator=(const LightBuffer&) = delete;


	/**
	* 新しいライトを追加する
	*
	* @param lightPtr ライトコンポーネントポインタ
	*/
	void Allocate(const LightPtr lightPtr);


private:
	/**
	* カメラに近いライトを選んでGPUメモリにコピーする
	*
	* @param cameraWorldMat	カメラのワールド座標変換行列
	*/
	void UpdateShaderLight(
		const mat4& cameraWorldMat);

	// SSBOのバインド
	void Bind();
	void Unbind();
};


#endif // LIGHTBUFFER_H_INCLUDE
