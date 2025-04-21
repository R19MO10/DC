/**
* @file Mesh.cpp
*/
#define _CRT_SECURE_NO_WARNINGS	/*sscanf関数でエラーにならないようにする*/
#include "MeshBuffer.h"

#include "../DebugLog.h"
#include "../Texture/TextureManager.h"
#include "../GraphicsObject/VertexArrayObject.h"
#include "../../Library/MikkTSpace/mikktspace.h"
#include <numeric>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdio.h>


/**
* MikkTSpace用のユーザーデータ
*/
struct UserData
{
	std::vector<Vertex>& vertices;
	std::vector<uint16_t>& indices;
};

/**
* MikkTSpaceのインターフェイス実装
*/
class MikkTSpace
{
public:
	// コンストラクタ
	MikkTSpace()
	{
		// モデルの総ポリゴン数を返す（総ポリゴン数はインデックス数の1/3）
		interface.m_getNumFaces = [](const SMikkTSpaceContext* pContext)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				return static_cast<int>(p->indices.size() / 3);
			};

		// ポリゴンの頂点数を返す
		interface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* pContext,
			int iFace)
			{
				return 3;	/*今回は三角形しか使わないため常に3を返す*/
			};

		// 頂点の座標を返す（fvPosOut：座標配列）
		interface.m_getPosition = [](const SMikkTSpaceContext* pContext,
			float fvPosOut[], int iFace, int iVert)
			{
				/*iFace番目のポリゴンのiVert番目の頂点の座標を、fvPosOut配列にX,Y,Zの順に代入する*/
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				fvPosOut[0] = p->vertices[index].position.x;
				fvPosOut[1] = p->vertices[index].position.y;
				fvPosOut[2] = p->vertices[index].position.z;
			};

		// 頂点の法線を返す（fvNormOut：法線配列）
		interface.m_getNormal = [](const SMikkTSpaceContext* pContext,
			float fvNormOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				fvNormOut[0] = p->vertices[index].normal.x;
				fvNormOut[1] = p->vertices[index].normal.y;
				fvNormOut[2] = p->vertices[index].normal.z;
			};

		// 頂点のテクスチャ座標を返す（fvTexOut：テクスチャ座標 ※二次元）
		interface.m_getTexCoord = [](const SMikkTSpaceContext* pContext,
			float fvTexcOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				fvTexcOut[0] = p->vertices[index].texcoord.x;
				fvTexcOut[1] = p->vertices[index].texcoord.y;
			};

		// 「タンジェントベクトル」と「バイタンジェントベクトルの向き」を受け取る
		// 
		//@fvTangent(エフブイ・タンジェント) : タンジェントベクトル
		//@fSign(エフ・サイン) : バイタンジェントベクトルの向き
		//@iFace(アイ・フェイス) : データを設定するポリゴンの番号
		//@iVert(アイ・バート) : データを設定する頂点の番号
		interface.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext,
			const float fvTangent[], float fSign, int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				p->vertices[index].tangent =
					vec4(fvTangent[0], fvTangent[1], fvTangent[2], fSign);
			};

		// 使わないのでnullptrを設定
		interface.m_setTSpace = nullptr;
	}

	// デストラクタ
	~MikkTSpace() = default;

	// メンバ変数
	SMikkTSpaceInterface interface;
};


/**
* MTLファイルを読み込む
*
* @param foldername OBJファイルのあるフォルダ名
* @param filename   MTLファイル名
*
* @return MTLファイルに含まれるマテリアルの配列
*/
std::vector<MaterialPtr> MeshBuffer::LoadMTL(
	const std::string& foldername, const char* filename)
{
	// MTLファイルを開く
	const std::string fullpath = foldername + filename;
	std::ifstream file(fullpath);
	if (!file) {
		LOG_ERROR("%sを開けません", fullpath.c_str());
		return {};
	}

	// MTLファイルを解析する
	std::vector<MaterialPtr> materials;
	MaterialPtr pMaterial;
	vec3 specularColor = vec3(1); // スペキュラ色
	float specularPower = 12;     // スペキュラ係数
	while (!file.eof()) {
		std::string line;
		std::getline(file, line);

		// マテリアル定義の読み取りを試みる
		char name[1000] = { 0 };
		if (sscanf(line.data(), " newmtl %[^\n]s", name) == 1) {
			if (pMaterial) {
				// スペキュラパラメータからラフネスを計算
				// NOTE: シェーダで法線分布項を計算するときに分母が0にならないようにするため、ラフネスは0より大きくなくてはならない
				specularPower *=
					std::max(std::max(specularColor.x, specularColor.y), specularColor.z);
				pMaterial->roughness = std::clamp(1 - log2(specularPower) / 12, 0.0001f, 1.0f);

				// スペキュラパラメータを初期値に戻す
				specularColor = vec3(1);
				specularPower = 12;

				// テクスチャが設定されていないマテリアル場合、White.tgaを設定しておく
				if (!pMaterial->texBaseColor) {
					pMaterial->texBaseColor = TextureManager::GetTexture("White.tga");
				}
			}
			pMaterial = std::make_shared<Material>();
			pMaterial->name = name;
			materials.push_back(pMaterial);
			continue;
		}

		// マテリアルが定義されていない場合は行を無視する
		if (!pMaterial) {
			continue;
		}

		// 基本色の読み取りを試みる
		if (sscanf(line.data(), " Kd %f %f %f",
			&pMaterial->baseColor.x, &pMaterial->baseColor.y, 
			&pMaterial->baseColor.z) == 3) {
			continue;
		}

		// 不透明度の読み取りを試みる
		if (sscanf(line.data(), " d %f", &pMaterial->baseColor.w) == 1) {
			continue;
		}

		// 基本色テクスチャ名の読み取りを試みる
		char textureName[1000] = { 0 };
		if (sscanf(line.data(), " map_Kd %[^\n]s", &textureName) == 1) {
			const std::string filename = foldername + textureName;
			if (std::filesystem::exists(filename)) {
				pMaterial->texBaseColor =
					TextureManager::GetTexture(filename.c_str());
			}
			else {
				LOG_WARNING("%sを開けません", filename.c_str());
			}
			continue;
		}

		// 発光色の読み取りを試みる
		if (sscanf(line.data(), " Ke %f %f %f",
			&pMaterial->emission.x, &pMaterial->emission.y,
			&pMaterial->emission.z) == 3) {
			continue;
		}

		// 発光色テクスチャ名の読み取りを試みる
		if (sscanf(line.data(), " map_Ke %[^\n]s", &textureName) == 1) {
			const std::string filename = foldername + textureName;
			if (std::filesystem::exists(filename)) {
				pMaterial->texEmission =
					TextureManager::GetTexture(filename.c_str());
			}
			else {
				LOG_WARNING("%sを開けません", filename.c_str());
			}
			continue;
		}

		// 法線テクスチャ名の読み取りを試みる
		if (sscanf(line.data(), " map_%*[Bb]ump %[^\n]s", &textureName) == 1) {
			const std::string filename = foldername + textureName;
			if (std::filesystem::exists(filename)) {
				pMaterial->texNormal =
					TextureManager::GetTexture(filename.c_str());
			}
			else {
				LOG_WARNING("%sを開けません", filename.c_str());
			}
			continue;
		}

		// スペキュラ色の読み取りを試みる
		if (sscanf(line.data(), " Ks %f %f %f",
			&specularColor.x, &specularColor.y, &specularColor.z) == 3) {
			continue;
		}

		// スペキュラ係数の読み取りを試みる
		if (sscanf(line.data(), " Ns %f", &specularPower) == 1) {
			continue;
		}

		// メタリックの読み取りを試みる
		if (sscanf(line.data(), " Pm %f", &pMaterial->metallic) == 1) {
			continue;
		}
	}

	// 最後のマテリアルのスペキュラパラメータを設定
	if (pMaterial) {
		// スペキュラパラメータからラフネスを計算
		specularPower *= std::max(std::max(specularColor.x, specularColor.y), specularColor.z);
		pMaterial->roughness = std::clamp(1 - log2(specularPower) / 12, 0.000000001f, 1.0f);

		// テクスチャが設定されていないマテリアル場合、White.tgaを設定しておく
		if (!pMaterial->texBaseColor) {
			pMaterial->texBaseColor = TextureManager::GetTexture("White.tga");
		}
	}

	// 読み込んだマテリアルの配列を返す
	return materials;
}

/**
* コンストラクタ
*/
MeshBuffer::MeshBuffer(size_t bufferSize)
	:primitiveBuffer(bufferSize)
{
	// タンジェント空間計算用のオブジェクトを作成する
	mikkTSpace = std::make_shared<MikkTSpace>();
	if (!mikkTSpace) {
		LOG_ERROR("MikkTSpaceの作成に失敗");
	}

	// スタティックメッシュの容量を予約
	meshes.reserve(100);
}

/**
* OBJファイルを読み込む
*
* @param filename OBJファイル名
*
* @return filenameから作成したメッシュ
*/
StaticMeshPtr MeshBuffer::LoadOBJ(const char* filename)
{
	// 以前に読み込んだファイルなら、作成済みのメッシュを返す
	{
		auto itr = meshes.find(filename);
		if (itr != meshes.end()) {	/*キーと位置するデータがある場合*/
			return itr->second;
		}
	}/*itr変数を後続のプログラムから見えなくするため*/

	// OBJファイルを開く
	std::ifstream file(filename);
	if (!file) {
		LOG_ERROR("%sを開けません", filename);
		return nullptr;
	}

	// フォルダ名を取得する
	std::string foldername(filename);
	{
		const size_t p = foldername.find_last_of("￥￥/");
		if (p != std::string::npos) {
			foldername.resize(p + 1);
		}
	}

	// OBJファイルを解析して、頂点データとインデックスデータを読み込む
	std::vector<vec3> positions;
	std::vector<vec2> texcoords;
	std::vector<vec3> normals;
	struct IndexSet { int v, vt, vn; };
	std::vector<IndexSet> faceIndexSet;	/*インデックスデータの組*/

	positions.reserve(20'000);
	texcoords.reserve(20'000);
	normals.reserve(20'000);
	faceIndexSet.reserve(20'000 * 3);	/*頂点数と同じ数の三角形を扱うには3倍する必要がある*/

	// マテリアル
	std::vector<MaterialPtr> materials;
	materials.reserve(100);

	// マテリアルの使用範囲
	struct UseMaterial {
		std::string name;   // マテリアル名
		size_t startOffset; // 割り当て範囲の先頭位置
	};
	std::vector<UseMaterial> usemtls;
	usemtls.reserve(100);

	// 仮データを追加(マテリアル指定がないファイル対策)
	usemtls.push_back({ std::string(), 0 });


	while (!file.eof()) {
		std::string line;
		std::getline(file, line);	/*一行ずつ読み取る*/
		const char* p = line.c_str();	/*sscanfに渡すためにconst char*に変換*/

		// 頂点座標の読み取りを試みる
		vec3 v;
		if (sscanf(p, " v %f %f %f", &v.x, &v.y, &v.z) == 3) {	/*書式指定文字列の先頭に「空白」を作る（行頭の空白やタブを飛ばしてくれるため）*/
			positions.push_back(v);								/*%nの効果「%nが現れるまでに読み取った文字数を引数に格納する」*/
			continue;
		}

		// テクスチャ座標の読み取りを試みる
		vec2 vt;
		if (sscanf(p, " vt %f %f", &vt.x, &vt.y) == 2) {	/*書式指定文字列の先頭に「空白」を作る*/
			texcoords.push_back(vt);
			continue;
		}

		// 法線の読み取りを試みる
		vec3 vn;
		if (sscanf(p, " vn %f %f %f", &vn.x, &vn.y, &vn.z) == 3) {
			normals.push_back(vn);
			continue;
		}

		// インデックスデータの読み取りを試みる
		IndexSet f0, f1, f2;
		int readByte;

		// 頂点座標+テクスチャ座標+法線
		if (sscanf(p, " f %u/%u/%u %u/%u/%u%n",
			&f0.v, &f0.vt, &f0.vn, &f1.v, &f1.vt, &f1.vn, &readByte) == 6) {
			p += readByte; // 読み取り位置を更新（読み取った文字数を加算）
			for (;;) {
				if (sscanf(p, " %u/%u/%u%n", &f2.v, &f2.vt, &f2.vn, &readByte) != 3) {
					break;
				}
				p += readByte; // 読み取り位置を更新（読み取った文字数を加算）
				faceIndexSet.push_back(f0);
				faceIndexSet.push_back(f1);
				faceIndexSet.push_back(f2);
				f1 = f2; // 次の三角形のためにデータを移動
			}
			continue;
		}

		// 頂点座標+テクスチャ座標
		if (sscanf(p, " f %u/%u %u/%u%n",	/*(法線がある場合は/の位置が異なるため、ifは成り立たなくなる)*/
			&f0.v, &f0.vt, &f1.v, &f1.vt, &readByte) == 4) {
			f0.vn = f1.vn = 0; // 法線なし
			p += readByte; // 読み取り位置を更新（読み取った文字数を加算）
			for (;;) {
				if (sscanf(p, " %u/%u%n", &f2.v, &f2.vt, &readByte) != 2) {
					break;
				}
				f2.vn = 0; // 法線なし
				p += readByte; // 読み取り位置を更新（読み取った文字数を加算）
				faceIndexSet.push_back(f0);
				faceIndexSet.push_back(f1);
				faceIndexSet.push_back(f2);
				f1 = f2; // 次の三角形のためにデータを移動	

				/*f0を基準とする三角形を取得している*/
			}
			continue;
		}

		// MTLファイルの読み取りを試みる
		char mtlFilename[1000];
		if (sscanf(line.data(), " mtllib %999s", mtlFilename) == 1) {
			const auto tmp = LoadMTL(foldername, mtlFilename);
			materials.insert(materials.end(), tmp.begin(), tmp.end());	/*マテリアル配列に追加する*/
			continue;
		}

		// 使用マテリアル名の読み取りを試みる
		char mtlName[1000];
		if (sscanf(line.data(), " usemtl %999s", mtlName) == 1) {
			usemtls.push_back({ mtlName, faceIndexSet.size() });
			continue;
		}

	} // while eof

	// 末尾に番兵を追加	/*「次のマテリアルがある」ということ前提でプログラムを書ける*/
	usemtls.push_back({ std::string(), faceIndexSet.size() });


	// OBJファイルのf構文とOpenGLの頂点インデックス配列の対応表
	/* v, vn, vtをそれぞれ20bitで十分なため「20bit×3 = 60bit」なので uint64_t
	   連想配列の値は頂点インデックスなのでuint16_t */
	std::unordered_map<uint64_t, uint16_t> indexMap;
	indexMap.reserve(10'000);


	std::vector<Vertex> vertices;
	vertices.reserve(faceIndexSet.size());
	std::vector<uint16_t> indices;
	indices.reserve(faceIndexSet.size());

	// 読み込んだデータを、OpenGLで使えるデータに変換
	for (const auto& e : faceIndexSet) {

		// f構文の値を64ビットの「キー」に変換
		const uint64_t key = static_cast<uint64_t>(e.v) +
			(static_cast<uint64_t>(e.vt) << 20) + (static_cast<uint64_t>(e.vn) << 40);

		// 対応表からキーに一致するデータを検索
		const auto itr = indexMap.find(key);
		if (itr != indexMap.end()) {
			// 対応表にあるので既存の頂点インデックスを使う
			indices.push_back(itr->second);
		}
		else {
			// 対応表にないので新しい頂点データを作成し、頂点配列に追加
			Vertex v;
			v.position = positions[e.v - 1];
			v.texcoord = texcoords[e.vt - 1];
			/*-1しているのはOBJファイルのインデックスは1から始まるのに対し、C++言語の添字は0からだから*/

			 // 法線が設定されていない場合は0を設定(あとで計算)
			if (e.vn == 0) {
				v.normal = { 0, 0, 0 };
			}
			else {
				v.normal = normals[e.vn - 1];
			}

			vertices.push_back(v);

			// 新しい頂点データのインデックスを、頂点インデックス配列に追加
			const uint16_t index = static_cast<uint16_t>(vertices.size() - 1);
			indices.push_back(index);

			// キーと頂点インデックスのペアを対応表に追加
			indexMap.emplace(key, index);
		}
	}

	// タンジェントベクトルを計算
	if (mikkTSpace) {
		// MikkTSpaceライブラリを使ってタンジェントを計算
		UserData userData = { vertices, indices };
		SMikkTSpaceContext context = { &mikkTSpace->interface, &userData };
		genTangSpaceDefault(&context);
	}
	else {
		// 手動でタンジェントを計算
		for (size_t i = 0; i < indices.size(); ++i) {
			// 頂点座標とテクスチャ座標の辺ベクトルを計算	

			//ポリゴンを構成する頂点インデックスのオフセット
			const size_t offset = (i / 3) * 3;
			const int i0 = indices[(i + 0) % 3 + offset];
			const int i1 = indices[(i + 1) % 3 + offset];
			const int i2 = indices[(i + 2) % 3 + offset];

			Vertex& v0 = vertices[i0];
			Vertex& v1 = vertices[i1];
			Vertex& v2 = vertices[i2];
			vec3 e1 = v1.position - v0.position; // 座標ベクトル
			vec3 e2 = v2.position - v0.position; // 座標ベクトル
			vec2 uv1 = v1.texcoord - v0.texcoord; // テクスチャ座標ベクトル
			vec2 uv2 = v2.texcoord - v0.texcoord; // テクスチャ座標ベクトル

			// テクスチャの面積がほぼ0の場合は無視
			float uvArea = uv1.x * uv2.y - uv1.y * uv2.x; // uvベクトルによる平行四辺形の面積
			if (abs(uvArea) < 0x1p-20) { // 十六進浮動小数点数リテラル(C++17)
				continue;
			}

			// テクスチャが裏向きに貼られている(面積が負の)場合、バイタンジェントベクトルの向きを逆にする
			if (uvArea >= 0) {
				v0.tangent.w = 1; // 表向き
			}
			else {
				v0.tangent.w = -1;// 裏向き
			}

			// タンジェント平面と平行なベクトルにするため、法線方向の成分を除去
			e1 -= v0.normal * dot(e1, v0.normal);
			e2 -= v0.normal * dot(e2, v0.normal);

			// タンジェントベクトルを計算
			vec3 tangent = normalize((uv2.y * e1 - uv1.y * e2) * v0.tangent.w);

			// 辺ベクトルのなす角を重要度としてタンジェントを合成
			float angle = acos(dot(e1, e2) / (length(e1) * length(e2)));
			v0.tangent += vec4(tangent * angle, 0);

			// タンジェントベクトルを正規化（平均値を出すている）
			for (auto& e : vertices) {
				vec3 t = normalize(vec3(e.tangent.x, e.tangent.y, e.tangent.z));
				e.tangent = vec4(t.x, t.y, t.z, e.tangent.w);
			}
		}
	} // mikkTSpace

	// 設定されていない法線を補う	
	/*FillMissingNormals(vertices.data(), vertices.size(),
		indices.data(), indices.size());*/

		// 変換したデータをバッファに追加
	primitiveBuffer.AddPrimitive(
		vertices.data(), vertices.size() * sizeof(Vertex),
		indices.data(), indices.size() * sizeof(uint16_t));

	// メッシュを作成
	const auto& pMesh = std::make_shared<StaticMesh>();

	// データの位置を取得
	const void* indexOffset = primitiveBuffer.GetPrimitiveBack().indices;
	const GLint baseVertex = primitiveBuffer.GetPrimitiveBack().baseVertex;


	// マテリアルに対応した描画パラメータを作成
	// 仮データと番兵以外のマテリアルがある場合、仮データを飛ばす
	size_t i = 0;
	if (usemtls.size() > 2) {
		i = 1; // 仮データと番兵以外のマテリアルがある場合、仮データを飛ばす
	}

	//最後のマテリアルには「次のマテリアル」がないので例外処理を行う
	for (; i < usemtls.size() - 1; ++i) {
		const UseMaterial& cur = usemtls[i]; // 使用中のマテリアル
		const UseMaterial& next = usemtls[i + 1]; // 次のマテリアル
		if (next.startOffset == cur.startOffset) {
			continue; // インデックスデータがない場合は飛ばす
		}

		// 描画パラメータを作成
		Primitive prim;
		prim.mode = GL_TRIANGLES;
		prim.count = static_cast<GLsizei>(next.startOffset - cur.startOffset);
		prim.indices = indexOffset;
		prim.baseVertex = baseVertex;
		prim.materialNo = 0; // デフォルト値を設定
		for (int i = 0; i < materials.size(); ++i) {
			if (materials[i]->name == cur.name) {
				prim.materialNo = i; // 名前の一致するマテリアルを設定
				break;
			}
		}
		pMesh->primitives.push_back(prim);

		// インデックスオフセットを変更
		indexOffset = reinterpret_cast<void*>(
			reinterpret_cast<size_t>(indexOffset) + sizeof(uint16_t) * prim.count);
	}

	// マテリアル配列が空の場合、デフォルトマテリアルを追加
	if (materials.empty()) {
		pMesh->materials.push_back(std::make_shared<Material>());
	}
	else {
		pMesh->materials.assign(materials.begin(), materials.end());	/*マテリアルをメッシュにコピー*/
	}

	// objファイル名を抽出する
	// '/' を区切り文字として文字列を分割
	std::string fname = filename;
	size_t lastSlash = fname.find_last_of("/");
	std::string objfile = fname.substr(lastSlash + 1);	// 例：box.obj

	pMesh->name = objfile;
	meshes.emplace(objfile, pMesh);	/*データに追加*/

	LOG("%sを読み込みました(頂点数=%d, インデックス数=%d)",
		filename, vertices.size(), indices.size());


	// 作成したメッシュを返す
	return pMesh;
}

/**
* 全ての頂点データを削除
*/
void MeshBuffer::Clear()
{
	meshes.clear();
	primitiveBuffer.Clear();
}


// 欠けている法線を補う
void FillMissingNormals(
	Vertex* vertices, size_t vertexCount,
	const uint16_t* indices, size_t indexCount)
{
	// 法線が設定されていない頂点を見つける
	std::vector<bool> missingNormals(vertexCount, false);
	for (int i = 0; i < vertexCount; ++i) {
		// 法線の長さが0の場合を「設定されていない」とみなす
		const vec3& n = vertices[i].normal;
		if (n.x == 0 && n.y == 0 && n.z == 0) {
			missingNormals[i] = true;	/*設定されていない*/
		}
	}

	// 法線を計算（3つの頂点ベクトルの平均から法線ベクトルを求める）
	for (int i = 0; i < indexCount; i += 3) {
		// 面を構成する2辺a, bを求める
		const int i0 = indices[i + 0];
		const int i1 = indices[i + 1];
		const int i2 = indices[i + 2];

		// いずれかの頂点がの法線が設定されていなかったら計算する
		if (missingNormals[i0] ||
			missingNormals[i1] ||
			missingNormals[i2])
		{
			const vec3& v0 = vertices[i0].position;	/*それぞれの頂点座標を得る*/
			const vec3& v1 = vertices[i1].position;
			const vec3& v2 = vertices[i2].position;
			const vec3 a = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };	/*v0からv1に向かうベクトル*/
			const vec3 b = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };	/*v0からv2に向かうベクトル*/

			// 外積によってaとbに垂直なベクトル(法線)を求める
			const float cx = a.y * b.z - a.z * b.y;
			const float cy = a.z * b.x - a.x * b.z;
			const float cz = a.x * b.y - a.y * b.x;

			// 法線を正規化して単位ベクトルにする	/*各ベクトルの成分をベクトルの長さで割る*/
			const float l = sqrt(cx * cx + cy * cy + cz * cz);
			const vec3 normal = { cx / l, cy / l, cz / l };

			// 法線が設定されていない頂点にだけ法線を加算
			if (missingNormals[i0]) {
				vertices[i0].normal += normal;
			}
			if (missingNormals[i1]) {
				vertices[i1].normal += normal;
			}
			if (missingNormals[i2]) {
				vertices[i2].normal += normal;
			}
		}
	}

	// 全部の計算が終わったら改めて法線を正規化
	for (int i = 0; i < vertexCount; ++i) {
		if (missingNormals[i]) {
			vec3& n = vertices[i].normal;
			const float l = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
			n = { n.x / l, n.y / l, n.z / l };
		}
	} // for i
}

// スタティックメッシュを取得
StaticMeshPtr MeshBuffer::GetStaticMesh(const char* name) const
{
	const auto& itr = meshes.find(MyFName(name));
	if (itr != meshes.end()) {
		//ファイル名に対応するスタティックメッシュを返す
		return itr->second;
	}

	LOG_ERROR("%sが見つかりませんでした", name);
	return nullptr;
}


VertexArrayObjectPtr MeshBuffer::GetVAO() const
{
	return primitiveBuffer.GetVAO();
}

/**
* 既存の描画パラメータとテクスチャから新しいスタティックメッシュを作成する
*/
StaticMeshPtr MeshBuffer::CreateStaticMesh(const char* name,
	const Primitive& prim, const TexturePtr& texBaseColor)
{
	auto p = std::make_shared<StaticMesh>();
	p->name = name;
	p->primitives.push_back(prim);
	p->primitives[0].materialNo = 0;
	p->materials.push_back(std::make_shared<Material>());
	if (texBaseColor) {
		p->materials[0]->texBaseColor = texBaseColor;
	}
	meshes.emplace(name, p);
	return p;
}

/**
* XY平面のプレートメッシュを作成する
*/
StaticMeshPtr MeshBuffer::CreatePlaneXY(const char* name)
{
	const Vertex vertexData[] = {
		{{-1.0f,-1.0f, 0}, {0, 1}},
		{{ 1.0f,-1.0f, 0}, {1, 1}},
		{{ 1.0f, 1.0f, 0}, {1, 0}},
		{{-1.0f, 1.0f, 0}, {0, 0}},
	};

	const uint16_t indexData[] = {
		0, 1, 2, 2, 3, 0,
	};

	// 法線を設定するために図形データのコピーを作る
	auto pVertex = static_cast<const Vertex*>(vertexData);
	auto pIndex = static_cast<const uint16_t*>(indexData);
	std::vector<Vertex> v(pVertex, pVertex + sizeof(vertexData) / sizeof(Vertex));

	// コピーした図形データに法線を設定
	for (auto& e : v) {
		e.normal = { 0, 0, 1 };
		e.tangent = { -1, 0, 0, 1 };
	}
	//FillMissingNormals(v.data(), v.size(), pIndex, sizeof(indexData) / sizeof(uint16_t));

	// バッファに追加
	primitiveBuffer.AddPrimitive(vertexData, sizeof(vertexData), indexData, sizeof(indexData));

	// 直前のAddPrimitiveで作成した描画パラメータを取得
	const Primitive& drawPrim =
		primitiveBuffer.GetPrimitive(primitiveBuffer.GetPrimitivesCount() - 1);

	return CreateStaticMesh(name, drawPrim, nullptr);
}
