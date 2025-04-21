/**
* @file Mesh.h
*/
#ifndef MESHBUFFER_H_INCLUDED
#define MESHBUFFER_H_INCLUDED
#include "StaticMesh.h"
#include "../../Utility/MyFName.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

// 先行宣言
class MeshBuffer;
using MeshBufferPtr = std::shared_ptr<MeshBuffer>;
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

/**
* 読み込んだStaticMeshをを管理するクラス
*/
class MeshBuffer
{
private:
	std::shared_ptr<class MikkTSpace> mikkTSpace;	//MikkTSpaceクラス
	std::unordered_map<MyFName, StaticMeshPtr> meshes;
	PrimitiveBuffer primitiveBuffer;

private:
	std::vector<MaterialPtr> LoadMTL(
		const std::string& foldername, const char* filename);

public:
	/**
	* メッシュバッファを作成する
	*
	* @param bufferSize_byte 格納できる頂点データのサイズ(バイト数)
	*/
	static MeshBufferPtr Create(size_t bufferSize)
	{
		return std::make_shared<MeshBuffer>(bufferSize);
	}

	// コンストラクタ・デストラクタ
	MeshBuffer(size_t bufferSize);
	~MeshBuffer() = default;

	// コピーと代入を禁止
	MeshBuffer(const MeshBuffer&) = delete;
	MeshBuffer& operator=(const MeshBuffer&) = delete;

	/**
　  * OBJファイルを読み込む
　  *
　  * @param filename OBJファイル名
　  *
　  * @return filenameから作成したメッシュ
　  */
	StaticMeshPtr LoadOBJ(const char* filename);

	// 全ての頂点データを削除
	void Clear();

	/**
	* スタティックメッシュの取得
	*
	* @param name スタティックメッシュの名前
	*
	* @return 名前がnameと一致するスタティックメッシュ
	*/
	StaticMeshPtr GetStaticMesh(const char* name) const;

	// VAOの取得
	VertexArrayObjectPtr GetVAO() const;

	/**
	* 既存の描画パラメータとテクスチャから新しいスタティックメッシュを作成する
	*
	* @param name         メッシュ名
	* @param params       メッシュの元になる描画パラメータ
	* @param texBaseColor メッシュに設定するベースカラーテクスチャ
	*
	* @return 作成したスタティックメッシュ
	*/
	StaticMeshPtr CreateStaticMesh(const char* name,
		const Primitive& prim, const TexturePtr& texBaseColor);

	// XY平面のプレートメッシュを作成する
	StaticMeshPtr CreatePlaneXY(const char* name);
};

/**
* 欠けている法線を補う
*
* @param vertices    頂点配列
* @param vertexCount 頂点配列の要素数
* @param indices     インデックス配列
* @param indexCount  インデックス配列の要素数
*/
void FillMissingNormals(
	Vertex* vertices, size_t vertexCount,
	const uint16_t* indices, size_t indexCount);

#endif // MESHBUFFER_H_INCLUDE