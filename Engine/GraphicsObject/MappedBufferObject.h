/**
* @file MappedBufferObject.h
*/
#ifndef MAPPEDBUFFEROBJECT_H_INCLUDED
#define MAPPEDBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include <memory>

// 先行宣言
class MappedBufferObject;
using MappedBufferObjectPtr = std::shared_ptr<MappedBufferObject>;

// SSBOのサイズを計算する
inline size_t CalcSsboSize(size_t n)
{
	const size_t alignment = 256; // アライメント(データの境界)
	return ((n + alignment - 1) / alignment) * alignment;
};

/**
* バッファオブジェクト(マップあり)
*/
class MappedBufferObject
{
private:
	GLuint id = 0;
	GLsizeiptr bufferSize = 0;

	uint8_t* p = nullptr;          // データコピー先アドレス
	size_t bufferIndex = 0;        // コピー先バッファのインデックス
	GLsync syncList[2] = { 0, 0 }; // 同期オブジェクト
	GLenum type = GL_ARRAY_BUFFER; // バッファの種類

public:
	/**
	*　バッファオブジェクトを作成する
	*
	* @param size        バッファオブジェクトのバイトサイズ
	* @param type        バッファオブジェクトの種類
	* @param access      以下のいずれかを指定する
	*                    GL_READ_WRITE 読み取りと書き込みの両対応でマップする
	*                    GL_READ_ONLY  読み取り専用としてマップする
	*                    GL_WRITE_ONLY 書き込み専用としてマップする
	*/
	static MappedBufferObjectPtr Create(GLsizeiptr size, GLenum type, GLenum access)
	{
		return std::make_shared<MappedBufferObject>(size, type, access);
	}

	MappedBufferObject(
		GLsizeiptr size, GLenum type, GLenum access);
	~MappedBufferObject();

	operator GLuint() const { return id; }
	GLsizeiptr GetSize() const { return bufferSize; }


	// GPUがバッファを使い終わるのを待つ
	void WaitSync();

	// データのコピー先アドレスを取得する
	uint8_t* GetMappedAddress() const;

	// 指定した範囲をOpenGLコンテキストに割り当てる
	void Bind(GLuint index, size_t offset, size_t size);

	// コピー先バッファを切り替える
	void SwapBuffers();

	GLenum GetType() const { return type; }
};

#endif // MAPPEDBUFFEROBJECT_H_INCLUDED