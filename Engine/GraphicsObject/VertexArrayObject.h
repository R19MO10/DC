/**
* @file VertexArrayObject.h
*/
#ifndef VERTEXARRAYOBJECT_H_INCLUDED
#define VERTEXARRAYOBJECT_H_INCLUDED
#include "glad/glad.h"
#include <memory>

// 先行宣言
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

/**
* 頂点配列オブジェクト(VAO)
*
* 頂点バッファに格納されている頂点データについて、
* 各要素がどのように配置されているかを記述するオブジェクト。
*/
class VertexArrayObject
{
private:
	GLuint id = 0; // オブジェクト管理番号

public:
	/**
	* VAOを作成する
	*
	* @return 作成したVAOへのポインタ
	*/
	static VertexArrayObjectPtr Create()
	{
		return std::make_shared<VertexArrayObject>();
	}

	// コンストラクタ、デストラクタ
	VertexArrayObject() { glCreateVertexArrays(1, &id); }
	~VertexArrayObject() { glDeleteVertexArrays(1, &id); }

	// コピーと代入を禁止
	VertexArrayObject(const VertexArrayObject&) = delete;
	VertexArrayObject& operator=(const VertexArrayObject&) = delete;

	// 管理番号を取得
	operator GLuint() const { return id; }

	/**
    * 頂点アトリビュートを設定
	*
	* @param index  設定する頂点アトリビュートの番号(0～15)
	* @param size   要素のデータ数(float=1, vec2=2, vec3=3, vec4=4)
	* @param stride 次の頂点データにある同一要素までの間隔(バイト数)
	* @param offset 頂点データ内における要素の位置(バイト数)
	*/
	inline void SetAttribute(
		GLuint index, GLint size, size_t stride, size_t offset)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE,
			static_cast<GLsizei>(stride), reinterpret_cast<void*>(offset));
	};

};


#endif // VERTEXARRAYOBJECT_H_INCLUDED