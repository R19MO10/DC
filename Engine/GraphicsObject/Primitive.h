/**
* @file Primitive.h
*/
#ifndef PRIMITIVE_INCLUDE_H
#define PRIMITIVE_INCLUDE_H

#include "glad/glad.h"
#include "../../Math/MyMath.h"

/**
* 頂点データ形式
*/
struct Vertex
{
	vec3 position;	// 頂点座標
	vec2 texcoord;	// テクスチャ座標
	vec3 normal;	// 法線
	vec4 tangent;	// タンジェント
};

/**
* プリミティブ
*/
struct Primitive
{
	GLenum mode = GL_TRIANGLES; // プリミティブの種類
	GLsizei count = 0;          // 描画するインデックス数
	const void* indices = 0;    // 描画開始インデックスのバイトオフセット
	GLint baseVertex = 0;       // インデックス0となる頂点配列内の位置
	int materialNo = -1;        // マテリアル
};

/**
* プリミティブを描画する
*/
inline void DrawPrimitive(const Primitive& prim)
{
	glDrawElementsInstancedBaseVertex(prim.mode, prim.count,
		GL_UNSIGNED_SHORT, prim.indices, 1, prim.baseVertex);
}

#endif // !PRIMITIVE_INCLUDE_H