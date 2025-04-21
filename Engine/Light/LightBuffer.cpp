/**
* @file LightBuffer.cpp
*/
#include "LightBuffer.h"

#include "../GameObject.h"
#include "../GraphicsObject/ProgramPipeline.h"
#include "../GraphicsObject/MappedBufferObject.h"
#include "../../Component/Light.h"
#include <GLFW/glfw3.h>
#include <algorithm>


/**
* コンストラクタ
*/
LightBuffer::LightBuffer()
{
	// ライト配列を予約
	lights.reserve(lightResizeCount);

	// SSBOを作成
	ssboLight = MappedBufferObject::Create(
		MaxShaderLightCount * sizeof(SSBOLightData),
		GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
}

/**
* 新しいライトを追加する
*
* @param lightPtr ライトコンポーネントポインタ
*/
void LightBuffer::Allocate(const LightPtr lightPtr)
{
	lights.push_back(lightPtr);
}


/**
* カメラに近いライトを選んでGPUメモリにコピーする
*
* @param cameraWorldMat	カメラのワールド座標変換行列
*/
void LightBuffer::UpdateShaderLight(
	const mat4& cameraWorldMat)
{
	// コピーするライトがなければライト数を0に設定
	if (lights.empty()) {
		ssboLight->WaitSync();
		uint8_t* p = ssboLight->GetMappedAddress();
		const int lightCount[4] = { 0 };
		memcpy(p, lightCount, sizeof(lightCount));
		return;
	}

	// ライトコンポーネントが削除されたら配列からも削除する
	const auto& unused_itr = std::remove_if(lights.begin(), lights.end(),
		[](const LightPtr lightPtr) { return lightPtr->IsDestroyed(); });
	lights.erase(unused_itr, lights.end());

	// カメラの座標
	const vec3 cameraPos = vec3(cameraWorldMat[3]);
	// カメラの正面ベクトル
	const vec3 cameraForward = { 
		cameraWorldMat[2].x,
		0,
		cameraWorldMat[2].z,
	};


	// カメラからライトまでの距離を計算
	std::vector<std::pair<float, const Light*>> distanceList;
	distanceList.reserve(lights.size());

	for (const auto& e : lights) {
		const auto& light = e;
		const GameObject* lightOwner = light->GetOwner();

		// 活動状況を調べる
		if (!lightOwner->CheckActive()) {
			continue;
		}
		if (!light->isActive) {
			continue;
		}

		// ライトの明るさが0以下なら飛ばす
		if (light->intensity <= 0) {
			continue;
		}

		// ライトの位置を取得
		const vec3 lightPos = vec3(lightOwner->GetWorldTransformMatrix()[3]);

		// カメラからライトへのベクトル
		const vec3 CameraToLightDirection = lightPos - cameraPos;

		// カメラの後ろにあるライトを除外
		if (dot(cameraForward, CameraToLightDirection) <= -light->radius) {
			continue;
		}

		// カメラからライトの半径までの距離
		const float d = length(CameraToLightDirection) - light->radius; 
		distanceList.push_back(std::make_pair(d, light.get()));
	}

	// 画面に影響するライトがなければライト数を0に設定
	if (distanceList.empty()) {
		ssboLight->WaitSync();
		uint8_t* p = ssboLight->GetMappedAddress();
		const int lightCount[4] = { 0 };
		memcpy(p, lightCount, sizeof(lightCount));
		return;
	}

	// カメラに近いライトを優先する
	std::stable_sort(distanceList.begin(), distanceList.end(),
		[&](const auto& a, const auto& b) { return a.first < b.first; });


	// ライトデータを要素ごとにまとめてGPUメモリにコピー
	const int lightCount = static_cast<int>(
		std::min(distanceList.size(), MaxShaderLightCount)); // コピーするライト数

	// コピーするライト配列を作成
	std::vector<SSBOLightData> copyList(lightCount);
	for (int i = 0; i < lightCount; ++i) {
		const Light* e = distanceList[i].second;
		const mat4 lightOwnerWorldMat = e->GetOwner()->GetWorldTransformMatrix();

		SSBOLightData data;
		data.position = vec3(lightOwnerWorldMat[3]);
		data.color = e->color * e->intensity;
		data.radius = e->radius;

		data.direction = vec3(0);
		data.coneAngle = 0;	// 照射角度が0以下ならポイントライト扱い
		data.falloffAngle = 0;

		// スポットライトの場合、スポットライトのパラメータを反映
		if (e->type == Light::Type::SpotLight) {
			data.direction = normalize(vec3(lightOwnerWorldMat[2]));
			data.coneAngle = e->coneAngle_rad;
			data.falloffAngle = e->falloffAngle_rad;
		}

		copyList[i] = data;
	}

	// ライトデータをGPUメモリにコピー
	ssboLight->WaitSync();
	uint8_t* p = ssboLight->GetMappedAddress();

	const int cnt[4] = { lightCount };
	memcpy(p, cnt, sizeof(cnt));
	p += sizeof(cnt);
	memcpy(p, copyList.data(), copyList.size() * sizeof(SSBOLightData));
}

/**
* SSBOをバインドする
*/
void LightBuffer::Bind()
{
	ssboLight->Bind(Bind::SSBO::Light, 0, ssboLight->GetSize());
}

/**
* SSBOのバインドを解除する
*/
void LightBuffer::Unbind()
{
	glBindBufferRange(
		ssboLight->GetType(), Bind::SSBO::Light, 0, 0, 0);
	ssboLight->SwapBuffers();
}
