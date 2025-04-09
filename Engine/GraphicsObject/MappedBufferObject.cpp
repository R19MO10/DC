/**
* @file MappedBufferObject.cpp
*/
#include "MappedBufferObject.h"

#include "../DebugLog.h"


/**
* コンストラクタ
*
* @param size        バッファオブジェクトのバイトサイズ
* @param type        バッファオブジェクトの種類
* @param access      以下のいずれかを指定する
*                    GL_READ_WRITE 読み取りと書き込みの両対応でマップする
*                    GL_READ_ONLY  読み取り専用としてマップする
*                    GL_WRITE_ONLY 書き込み専用としてマップする
*/
MappedBufferObject::MappedBufferObject(
	GLsizeiptr size, GLenum type, GLenum access) :
	type(type)
{
	// ダブルバッファのため、サイズを256バイト境界に繰り上げる
	bufferSize = ((size + 255) / 256) * 256;

	// アクセスの種類によってフラグを選択
	GLbitfield flags = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	switch (access) {
	default:
		LOG_ERROR("引数が間違っています(access=%d)", access);
		[[fallthrough]];
	case GL_READ_WRITE: 
		flags |= GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
		break;
	case GL_READ_ONLY:  
		flags |= GL_MAP_READ_BIT; 
		break;
	case GL_WRITE_ONLY: 
		flags |= GL_MAP_WRITE_BIT;
		break;
	}

	// バッファオブジェクトを作成
	glCreateBuffers(1, &id);
	glNamedBufferStorage(id, bufferSize * 2, // ダブルバッファ用に2倍確保
		nullptr, flags);

	// GPUメモリをCPUメモリアドレスにマップする
	p = static_cast<uint8_t*>(glMapNamedBuffer(id, access));
}

/**
* デストラクタ
*/
MappedBufferObject::~MappedBufferObject()
{
	glDeleteBuffers(1, &id);
}


/**
* GPUがバッファを使い終わるのを待つ
*/
void MappedBufferObject::WaitSync()
{
	GLsync& sync = syncList[bufferIndex];
	for (; sync;) {
		const GLuint64 timeout = 16'700'000; // 16.7ミリ秒(約1/60秒)
		const GLenum status = glClientWaitSync(sync, 0, timeout);
		if (status != GL_TIMEOUT_EXPIRED) {
			// 使い終わった同期オブジェクトは削除しなくてはならない
			glDeleteSync(sync);
			sync = 0;
			break;
		}
	} // for syncList
}

/**
* データのコピー先アドレスを取得する
*
* @return コピー先アドレス
*
* コピー可能な範囲はGetSize()で取得できる
*/
uint8_t* MappedBufferObject::GetMappedAddress() const
{
	return p + bufferIndex * bufferSize;
}

/**
* 指定した範囲をOpenGLコンテキストに割り当てる
*
* @param index  割り当てるバインディングポイント番号
* @param offset 割り当て範囲の先頭位置
* @param size   割り当て範囲のバイトサイズ
*/
void MappedBufferObject::Bind(
	GLuint index, size_t offset, size_t size)
{
	offset += bufferIndex * bufferSize;
	glBindBufferRange(type, index, id, offset, size);
}

/**
* コピー先バッファを切り替える
*/
void MappedBufferObject::SwapBuffers()
{
	syncList[bufferIndex] =
		glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	bufferIndex = (bufferIndex + 1) % 2;
}