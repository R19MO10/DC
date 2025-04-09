/**
* @file PrimitiveBuffer.h
*/
#ifndef PRIMITIVEBUFFER_H_INCLUDED
#define PRIMITIVEBUFFER_H_INCLUDED
#include "glad/glad.h"
#include "Primitive.h"
#include "../../Math/MyMath.h"
#include <memory>
#include <vector>

// 先行宣言
class PrimitiveBuffer;
using PrimitiveBufferPtr = std::shared_ptr<PrimitiveBuffer>;
class BufferObject;
using BufferObjectPtr = std::shared_ptr<BufferObject>;
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

/**
* プリミティブを管理するクラス
*/
class PrimitiveBuffer
{
private:
	std::vector<Primitive> primitives; // プリミティブ配列

	VertexArrayObjectPtr vao; // 頂点アトリビュート配列
	BufferObjectPtr buffer;   // 頂点データおよびインデックスデータ

	size_t usedBytes = 0;     // バッファの使用済み容量(バイト)

public:
	// プリミティブバッファを作成する
	static PrimitiveBufferPtr Create(size_t bufferSize) {
		return std::make_shared<PrimitiveBuffer>(bufferSize);
	}

	// コンストラクタ・デストラクタ
	PrimitiveBuffer(size_t bufferSize);
	~PrimitiveBuffer() = default;

	// コピーと代入を禁止
	PrimitiveBuffer(const PrimitiveBuffer&) = delete;
	PrimitiveBuffer& operator=(const PrimitiveBuffer&) = delete;

	/**
	* プリミティブの追加
	*
	* @param vertices    GPUメモリにコピーする頂点データ配列
	* @param vertexBytes verticesのバイト数
	* @param indices     GPUメモリにコピーするインデックスデータ配列
	* @param indexBytes  indicesのバイト数
	* @param mode        プリミティブの種類
	*/
	void AddPrimitive(const Vertex* vertices, size_t vertexBytes,
		const uint16_t* indices, size_t indexBytes, GLenum mode = GL_TRIANGLES);

	// プリミティブの取得
	inline const Primitive& GetPrimitive(size_t index) const { return primitives[index]; }

	// プリミティブ配列のサイズを取得
	inline const size_t GetPrimitivesCount() const { return primitives.size(); }

	// VAOの取得
	inline const VertexArrayObjectPtr& GetVAO() const { return vao; }

	// プリミティブ配列の一番後ろを返す
	inline const Primitive& GetPrimitiveBack() const { return primitives.back(); }


	// 全てのプリミティブを削除
	void Clear();
};

#endif // PRIMITIVEBUFFER_H_INCLUDED