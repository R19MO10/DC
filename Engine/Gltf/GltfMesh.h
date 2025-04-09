/**
* @ file GltfMesh.h
*/
#ifndef GLTFMESH_H_INCLUDED
#define GLTFMESH_H_INCLUDED
#include "../../Utility/MyFName.h"
#include "../../Math/MyMath.h"
#include <vector>
#include <string>
#include <memory>

// 先行宣言
class Texture;
using TexturePtr = std::shared_ptr<Texture>;
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;
struct GltfFile;
using  GltfFilePtr = std::shared_ptr<GltfFile>;


/**
* プリミティブデータ
*/
struct GltfPrimitive
{
	GLenum mode = GL_TRIANGLES; // プリミティブの種類
	GLsizei count = 0;          // 描画するインデックス数
	GLenum type = GL_UNSIGNED_SHORT; // インデックスデータ型
	const GLvoid* indices = 0;  // 描画開始インデックスのバイトオフセット
	GLint baseVertex = 0;       // インデックス0番とみなされる頂点配列内の位置

	VertexArrayObjectPtr vao;    // プリミティブ用VAO
	size_t materialNo = 0; // マテリアル番号
};

/**
* メッシュデータ
*/
struct GltfMesh
{
	MyFName name; // メッシュ名
	std::vector<GltfPrimitive> primitives; // プリミティブ配列
};

#endif // GLTFMESH_H_INCLUDED