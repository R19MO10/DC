/**
* @file GltfFileBuffer.h
*/
#ifndef GLTFFILEBUFFER_H_INCLUDED
#define GLTFFILEBUFFER_H_INCLUDED
#include "GltfFile.h"
#include <unordered_map>

// 先行宣言
class GltfFileBuffer;
using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;
class BufferObject;
using BufferObjectPtr = std::shared_ptr<BufferObject>;
class MappedBufferObject;
using MappedBufferObjectPtr = std::shared_ptr<MappedBufferObject>;


// メッシュ描画に使うSSBOの範囲
struct GltfFileRange {
	GLintptr offset;
	size_t size;
};

/**
* glTFファイルから読み込んだメッシュを管理するクラス
*/
class GltfFileBuffer
{
private:
	BufferObjectPtr buffer;         // binファイルを読み込むバッファ
	uint8_t* pBuffer = nullptr;     // マップされたアドレス
	GLsizeiptr curBufferSize = 0;   // 書き込み済みデータ数

	// ファイル管理用の連想配列
	std::unordered_map<MyFName, GltfFilePtr> files;

	// 姿勢行列の配列
	using AnimationMatrices = std::vector<mat4>;

	// アニメーションの姿勢行列バッファ
	MappedBufferObjectPtr animationBuffer;
	AnimationMatrices tmpAnimationBuffer;

public:
	/**
	* glTFファイルバッファを作成する
	* 
	* @param bufferCapacity ファイル格納用バッファの最大バイト数
	* @param maxMatrixCount アニメーション用SSBOに格納できる最大行列数
	*/
	static GltfFileBufferPtr Create(
		size_t bufferCapacity, size_t maxMatrixCount) {
		return std::make_shared<GltfFileBuffer>(bufferCapacity, maxMatrixCount);
	}

	// コンストラクタ・デストラクタ
	GltfFileBuffer(size_t bufferCapacity, size_t maxMatrixCount);
	~GltfFileBuffer() = default;

	// ファイルの読み込み
	GltfFilePtr LoadGltf(const char* filename);


	// glTFファイルを取得する
	GltfFilePtr GetGltfFile(const char* name) const;


	// アニメーションの姿勢行列バッファの管理
	void ClearAnimationBuffer();
	GltfFileRange AddAnimationMatrices(const AnimationMatrices& matBones);
	void UploadAnimationBuffer();
	void BindAnimationBuffer(GLuint bindingPoint, const GltfFileRange& range);
	void UnbindAnimationBuffer(GLuint bindingPoint);

private:
	GltfFilePtr Parse(const char* text, const char* foldername);
};

#endif // GLTFFILEBUFFER_H_INCLUDED