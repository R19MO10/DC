/**
* @file BufferObject.h
*/
#ifndef BUFFEROBJECT_H_INCLUDED
#define BUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include <memory>

// 先行宣言
class BufferObject;
using BufferObjectPtr = std::shared_ptr<BufferObject>;

/**
* バッファオブジェクト(BO)
*/
class BufferObject
{
private:
	GLuint id = 0;					// オブジェクト管理番号
	GLsizeiptr bufferSize_byte = 0;	// バッファサイズ(バイト数)

public:
	/**
	* バッファオブジェクトを作成する
	*
	* @param size  バッファオブジェクトのバイトサイズ
	* @param data  バッファにコピーするデータのアドレス
	*              nullptrを指定すると空のバッファが作成される
	* @param flags 用途を示すビットフラグの論理和(glBufferStorageを参照)
	*
	* @return 作成したバッファオブジェクトへのポインタ
	*/
	static BufferObjectPtr Create(GLsizeiptr size, const void* data = nullptr, GLbitfield flags = 0) {
		return std::make_shared<BufferObject>(size, data, flags);
	}

	// コンストラクタ
	BufferObject(GLsizeiptr size, const void* data, GLbitfield flags) {
		bufferSize_byte = size;
		glCreateBuffers(1, &id);
		glNamedBufferStorage(id, bufferSize_byte, data, flags);
	}

	// デストラクタ
	~BufferObject() { glDeleteBuffers(1, &id); }

	// コピーと代入を禁止
	BufferObject(const BufferObject&) = delete;
	BufferObject& operator=(const BufferObject&) = delete;

	// 管理番号を取得
	operator GLuint() const { return id; }

	// バッファサイズを取得
	GLsizeiptr GetSize() const { return bufferSize_byte; }
};

#endif // BUFFEROBJECT_H_INCLUDED