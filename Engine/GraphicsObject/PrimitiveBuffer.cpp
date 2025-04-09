/**
* @file PrimitiveBuffer.cpp
*/
#include "PrimitiveBuffer.h"

#include "../DebugLog.h"
#include "BufferObject.h"
#include "VertexArrayObject.h"
#include <numeric>


PrimitiveBuffer::PrimitiveBuffer(size_t bufferSize)
{
	// バッファオブジェクトを作成
	buffer = BufferObject::Create(bufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);

	// VAOを作成
	vao = VertexArrayObject::Create();

	// VBOとIBOをVAOにバインド
	glBindVertexArray(*vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);

	// 頂点アトリビュートを設定
	vao->SetAttribute(0, 3, sizeof(Vertex), offsetof(Vertex, position));
	vao->SetAttribute(1, 2, sizeof(Vertex), offsetof(Vertex, texcoord));
	vao->SetAttribute(2, 3, sizeof(Vertex), offsetof(Vertex, normal));
	vao->SetAttribute(3, 4, sizeof(Vertex), offsetof(Vertex, tangent));

	// 誤った操作が行われないようにバインドを解除
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// 描画パラメータの容量を予約
	primitives.reserve(100);
}


void PrimitiveBuffer::AddPrimitive(const Vertex* vertices, size_t vertexBytes,
	const uint16_t* indices, size_t indexBytes, GLenum mode)
{
	// 空き容量が足りていることを確認
	// 必要なバイト数が空きバイト数より大きい場合は追加できない
	const size_t totalBytes = vertexBytes + indexBytes;
	const size_t freeBytes = buffer->GetSize() - usedBytes;
	if (totalBytes > freeBytes) {
		LOG_ERROR("容量が足りません(要求サイズ=%d, 使用量/最大容量=%d/%d)",
			vertexBytes + indexBytes, usedBytes, buffer->GetSize());
		return;
	}

	// 頂点データをGPUメモリにコピー（VBOとIBOに同じバッファオブジェクトを指定している）
	GLuint tmp[2];
	glCreateBuffers(2, tmp);	/*一時的なバッファを作成*/
	glNamedBufferStorage(tmp[0], vertexBytes, vertices, 0);	/*一時バッファに書き込み（頂点データ用バッファ）*/
	glNamedBufferStorage(tmp[1], indexBytes, indices, 0);	/*一時バッファに書き込み（インデックスデータ用バッファ）*/

	//コピー
	/*(コピー元バッファの管理番号, コピー先バッファの管理番号, コピー元の読み取り開始位置, コピー先の書き込み開始位置, コピーするバイト数);*/
	glCopyNamedBufferSubData(tmp[0], *buffer, 0, usedBytes, vertexBytes);
	glCopyNamedBufferSubData(tmp[1], *buffer, 0, usedBytes + vertexBytes, indexBytes);

	glDeleteBuffers(2, tmp);	/*一時バッファを削除*/


	// 追加した図形の描画パラメータを作成
	Primitive newPrim;
	newPrim.mode = mode;
	newPrim.count = static_cast<GLsizei>(indexBytes / sizeof(uint16_t));
	// インデックスデータの位置(バイト数で指定)は頂点データの直後
	newPrim.indices = reinterpret_cast<void*>(usedBytes + vertexBytes);
	// 頂点データの位置は頂点データ数で指定する
	newPrim.baseVertex = static_cast<GLint>(usedBytes / sizeof(Vertex));

	primitives.push_back(newPrim);	 // 描画パラメータを配列に追加

	// 次のデータ格納開始位置を計算（インデックスデータと頂点データの最上公倍数(20byte)）
	constexpr size_t a = std::lcm(sizeof(uint16_t)/*インデックスデータ(2byte)*/, sizeof(Vertex)/*頂点データ(20byte)*/); // 共通の境界サイズ

	//「元の値以上かつ、最小の境界サイズの倍数」
	usedBytes += ((totalBytes + a - 1) / a) * a;	/*((元の値 + 境界サイズ - 1) / 境界サイズ) * 境界サイズ*/
}

/**
* 全てのプリミティブを削除する
*/
void PrimitiveBuffer::Clear()
{
	primitives.clear();
	usedBytes = 0;
}