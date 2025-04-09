/**
* @file GltfFileBuffer.cpp
*/
#include "GltfFileBuffer.h"

#include "../../UniformLocation.h"
#include "../DebugLog.h"
#include "../Texture/TextureManager.h"
#include "../GraphicsObject/BufferObject.h"
#include "../GraphicsObject/MappedBufferObject.h"
#include "../GraphicsObject/VertexArrayObject.h"
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "../../Library/nlohmann/json.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>

//「不要になったデータを開放する処理」は実装しません。必要に応じてみなさん自身が実装してください。
using json = nlohmann::json;


namespace {
	/**
	* バイナリデータ
	*/
	struct BinaryData
	{
		GLsizeiptr offset;     // GPUメモリ上のデータ開始オフセット
		std::vector<char> bin; // CPUメモリに読み込んだデータ
	};
	using BinaryList = std::vector<BinaryData>;
	/**
	* ファイルを読み込む
	*
	* @param filename 読み込むファイル名
	*
	* @return 読み込んだデータ配列
	*/
	std::vector<char> ReadFile(const char* filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file) {
			LOG_WARNING("%sを開けません", filename);
			return {};
		}
		std::vector<char> buf(std::filesystem::file_size(filename));
		file.read(buf.data(), buf.size());
		return buf;
	}


	/**
	* 頂点アトリビュート番号
	* VAOが管理している頂点アトリビュート配列のうち「何番目の頂点アトリビュートを操作するのか」を指定する
	*/
	enum AttribIndex
	{
		position,
		texcoord0,
		normal,
		tangent,
		joints0,
		weights0,
	};

	/**
	* 必要な頂点データ要素がプリミティブに存在しない場合に使うデータ
	*/
	struct DefaultVertexData
	{
		vec3 position = vec3(0);
		vec2 texcoord0 = vec2(0);
		vec3 normal = vec3(0, 0, -1);
		vec4 tangent = vec4(1, 0, 0, 1);
		vec4 joints0 = vec4(0);
		vec4 weights0 = vec4(0);
	};

	/**
	* 成分の数を取得する
	*
	* @param accessor パラメータを持つアクセサ
	*
	* @return 成分の数
	*/
	int GetComponentCount(const json& accessor)
	{
		// 型名と成分数の対応表
		static const struct {
			const char* type;   // 型の名前
			int componentCount; // 成分数
		} componentCountList[] = {
			{ "SCALAR", 1 },
			{ "VEC2", 2 }, { "VEC3", 3 }, { "VEC4", 4 },
			{ "MAT2", 4 }, { "MAT3", 9 }, { "MAT4", 16 },
		};
		// 対応表から成分数を取得
		const std::string& type = accessor["type"].get<std::string>();
		for (const auto& e : componentCountList) {
			if (type == e.type) {
				return e.componentCount;
			}
		}
		LOG_WARNING("不明な型`%s`が指定されています", type.c_str());
		return 1; // glTFの仕様どおりなら、ここに来ることはない
	}

	/**
	* ストライド(次の要素までの距離)を取得する
	*
	* @param accessor   アクセサ
	* @param bufferView バッファビュー
	*
	* @return ストライド
	*/
	GLsizei GetByteStride(const json& accessor, const json& bufferView)
	{
		// byteStrideが定義されていたら、その値を返す
		const auto byteStride = bufferView.find("byteStride");
		if (byteStride != bufferView.end()) {
			return byteStride->get<int>();
		}
		// byteStrideが未定義の場合、要素1個分のサイズをストライドとする
		// 参考: glTF-2.0仕様 3.6.2.4. Data Alignment
		int componentSize = 1; // 成分型のサイズ
		const int componentType = accessor["componentType"].get<int>();
		switch (componentType) {
		case GL_BYTE:           componentSize = 1; break;
		case GL_UNSIGNED_BYTE:  componentSize = 1; break;
		case GL_SHORT:          componentSize = 2; break;
		case GL_UNSIGNED_SHORT: componentSize = 2; break;
		case GL_UNSIGNED_INT:   componentSize = 4; break;
		case GL_FLOAT:          componentSize = 4; break;
		default:
			LOG_WARNING("glTFの仕様にない型%dが使われています", componentType);
			break;
		}
		// ストライド = 成分型のサイズ * 成分数
		const int componentCount = GetComponentCount(accessor);
		return componentSize * componentCount;
	}

	/**
	* データの開始位置を取得する
	*
	* @param accessor   アクセサ
	* @param bufferView バッファビュー
	* @param binaryList バイナリデータ配列
	*
	* @return データの開始位置
	*/
	GLsizeiptr GetBinaryDataOffset(const json& accessor,
		const json& bufferView, const BinaryList& binaryList)
	{
		const int bufferId = bufferView["buffer"].get<int>();
		const int byteOffset = accessor.value("byteOffset", 0);			// インデックスデータ内のオフセット
		const int baseByteOffset = bufferView.value("byteOffset", 0);	// バッファ内のオフセット
		return binaryList[bufferId].offset + baseByteOffset + byteOffset;
	}

	/**
	* CPU側のデータのアドレスを取得する
	*
	* @param accessor    アクセサ
	* @param bufferViews バッファビュー配列
	* @param binaryList  バイナリデータ配列
	*
	* @return データのアドレス
	*/
	const void* GetBinaryDataAddress(const json& accessor,
		const json& bufferViews, const BinaryList& binaryList)
	{
		const int bufferViewId = accessor["bufferView"].get<int>();
		const json& bufferView = bufferViews[bufferViewId];

		const int bufferId = bufferView["buffer"].get<int>();
		const int byteOffset = accessor.value("byteOffset", 0);
		const int baseByteOffset = bufferView.value("byteOffset", 0);
		return binaryList[bufferId].bin.data() + baseByteOffset + byteOffset;
	}

	/**
	* 頂点アトリビュートを設定する
	*
	* @param index			頂点アトリビュートのインデックス
	* @param key			頂点アトリビュートの名前
	* @param attributes		頂点アトリビュート
	* @param accessors		アクセサ
	* @param bufferViews	バッファビュー
	* @param binaryList		バイナリデータ配列
	*
	* @retval true  頂点アトリビュートを設定した
	* @retval false 頂点アトリビュートのパラメータがなかった
	*/
	bool SetVertexAttribute(
		GLuint index, const char* key,
		const json& attributes, const json& accessors,
		const json& bufferViews, const BinaryList& binaryList)
	{
		glEnableVertexAttribArray(index); // 頂点アトリビュートを有効化
		const auto attribute = attributes.find(key);
		if (attribute != attributes.end()) {
			// アクセサとバッファビューを取得
			const json& accessor = accessors[attribute->get<int>()];
			const int bufferViewId = accessor["bufferView"].get<int>();
			const json& bufferView = bufferViews[bufferViewId];

			// 頂点アトリビュートのパラメータを取得
			const GLint componentCount = GetComponentCount(accessor);
			const GLenum componentType = accessor["componentType"].get<int>();
			const GLboolean normalized = accessor.value("normalized", false);
			const GLsizei byteStride = GetByteStride(accessor, bufferView);
			const GLsizeiptr offset = GetBinaryDataOffset(accessor, bufferView, binaryList);

			// VAOに頂点アトリビュートを設定
			glVertexAttribPointer(index, componentCount,
				componentType, normalized, byteStride, reinterpret_cast<void*>(offset));

			return true;
		}
		return false;
	}

	/**
	* VAOにデフォルトの頂点アトリビュートを設定する
	*
	* @param index  設定先の頂点アトリビュート番号
	* @param size   データの要素数
	* @param offset データの位置
	* @param vbo    頂点データを保持するVBO
	*/
	void SetDefaultAttribute(GLuint index, GLint size, GLuint offset, GLuint vbo)
	{
		glVertexAttribFormat(index, size, GL_FLOAT, GL_FALSE, offset);
		glVertexAttribBinding(index, index);

		/* ストライドにゼロを指定できることですべての頂点インデックスに対して同じ頂点データが使われる */
		glBindVertexBuffer(index, vbo, 0, 0);
	}

	/**
	* テクスチャを読み込む
	*
	* @param objectName  テクスチャ情報のキー文字列
	* @param gltf        glTFファイルのJSONオブジェクト
	* @param parent      テクスチャ情報を持つJSONオブジェクト
	* @param foldername  glTFファイルがあるフォルダ名
	*
	* @return 読み込んだテクスチャ、またはtexDefault
	*/
	TexturePtr LoadTexture(
		const char* objectName, const json& gltf, const json& parent, const std::string& foldername)
	{
		// テクスチャ情報を取得
		const auto textureInfo = parent.find(objectName);
		if (textureInfo == parent.end()) {
			return nullptr;
		}

		// テクスチャ番号を取得
		const auto textures = gltf.find("textures");
		const int textureNo = textureInfo->at("index").get<int>();
		if (textures == gltf.end() || textureNo >= textures->size()) {
			return nullptr;
		}

		// イメージソース番号を取得
		const json& texture = textures->at(textureNo);
		const auto source = texture.find("source");
		if (source == texture.end()) {
			return nullptr;
		}

		// イメージ番号を取得
		const auto images = gltf.find("images");
		const int imageNo = source->get<int>();
		if (images == gltf.end() || imageNo >= images->size()) {
			return nullptr;
		}

		// ファイル名を取得
		const json& image = images->at(imageNo);
		const auto imageUri = image.find("uri");
		if (imageUri == image.end()) {
			return nullptr;
		}

		// フォルダ名を追加し、拡張子をtgaに変更
		std::filesystem::path uri = imageUri->get<std::string>();
		std::filesystem::path filename = foldername;
		filename /= uri.parent_path();	// フォルダ名
		filename /= uri.stem();			// テクスチャ名
		filename += ".tga";				// 拡張子をtgaに変更

		// テクスチャを読み込む
		return TextureManager::GetTexture(filename.string().c_str(), Usage::forGltf);
	}

	/**
	* JSONの配列データをvec3に変換する
	*
	* @param json 変換元となる配列データ
	*
	* @return jsonを変換してできたvec3の値
	*/
	vec3 GetVec3(const json& json)
	{
		if (json.size() < 3) {
			return vec3(0);
		}
		return {
			json[0].get<float>(),
			json[1].get<float>(),
			json[2].get<float>()
		};
	}

	/**
	* JSONの配列データをquatに変換する
	*
	* @param json 変換元となる配列データ
	*
	* @return jsonを変換してできたquatの値
	*/
	Quaternion GetQuat(const json& json)
	{
		if (json.size() < 4) {
			return { 0, 0, 0, 1 };
		}
		return {
			json[0].get<float>(),
			json[1].get<float>(),
			json[2].get<float>(),
			json[3].get<float>()
		};
	}

	/**
	* JSONの配列データをmat4に変換する
	*
	* @param json 変換元となる配列データ
	*
	* @return jsonを変換してできたmat4の値
	*/
	mat4 GetMat4(const json& json)
	{
		if (json.size() < 16) {
			return mat4(1);
		}
		mat4 m;
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				m[y][x] = json[y * 4 + x].get<float>();
			}
		}
		return m;
	}

	/**
	* ノードのローカル座標変換行列を計算する
	*
	* @param node gltfノード
	*
	* @return nodeのローカル座標変換行列
	*/
	mat4 GetLocalMatrix(const json& node)
	{
		// 行列データがある場合、行列データを読み取って返す
		const auto matrix = node.find("matrix");
		if (matrix != node.end()) {
			return GetMat4(*matrix);
		}

		// 行列データがない場合、
		// スケール→回転→平行移動の順で適用した行列を返す
		mat4 result(1);
		const auto t = node.find("translation");
		if (t != node.end()) {
			result[3] = vec4(GetVec3(*t), 1);
		}
		const auto r = node.find("rotation");
		if (r != node.end()) {
			result *= mat4(GetQuat(*r));
		}
		const auto s = node.find("scale");
		if (s != node.end()) {
			result *= Mat::Scale(GetVec3(*s));
		}
		return result;
	}

	/**
	* メッシュを持つノードをリストアップする
	*/
	void GetMeshNodeList(const GltfNode* node, std::vector<const GltfNode*>& list)
	{
		if (node->mesh >= 0) {
			// メッシュを持っている
			list.push_back(node);
		}
		for (const GltfNode* child : node->children) {
			// 全ての子ノードに対して行う
			GetMeshNodeList(child, list);
		}
	}

	/**
	* アニメーションチャネルを作成する
	*
	* @param pTimes       時刻の配列のアドレス
	* @param pValues      値の配列のアドレス
	* @param inputCount   配列の要素数
	* @param targetNodeId 値の適用対象となるノードID
	* @param interp       補間方法
	*
	* @return 作成したアニメーションチャネル
	*/
	template<typename T>
	GltfChannel<T> MakeAnimationChannel(
		const GLfloat* pTimes, const void* pValues, size_t inputCount,
		int targetNodeId, GltfInterpolation interp)
	{
		// 時刻と値の配列からキーフレーム配列を作成
		const T* pData = static_cast<const T*>(pValues);
		GltfChannel<T> channel;
		channel.keyframes.resize(inputCount);
		for (int i = 0; i < inputCount; ++i) {
			channel.keyframes[i] = { pTimes[i], pData[i] };
		}

		// 適用対象ノードIDと補間方法を設定
		channel.targetNodeId = targetNodeId;
		channel.interpolation = interp;

		return channel; // 作成したチャネルを返す
	}

} // unnamed namespace


/**
* コンストラクタ
*
* @param bufferCapacity ファイル格納用バッファの最大バイト数
* @param maxMatrixCount アニメーション用SSBOに格納できる最大行列数
*/
GltfFileBuffer::GltfFileBuffer(size_t bufferCapacity, size_t maxMatrixCount)
{
	// GPUメモリを確保し、書き込み専用としてマップする
	buffer = BufferObject::Create(bufferCapacity, nullptr,
		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	pBuffer = static_cast<uint8_t*>(glMapNamedBuffer(*buffer, GL_WRITE_ONLY));
	/* glTFはVBOとIBOに分けることはせず、
	   頂点データもインデックスデータも同じバッファオブジェクトに格納する */

	// バッファの先頭にダミーデータを設定
	const DefaultVertexData defaultData;
	memcpy(pBuffer, &defaultData, sizeof(defaultData));
	curBufferSize = sizeof(defaultData);
	/* テクスチャ座標などを持たないglTFファイルのVAOにこれらのデータを割り当てる */

	// アニメーションの姿勢行列用バッファを作成
	animationBuffer = MappedBufferObject::Create(maxMatrixCount * sizeof(mat4),
		GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	tmpAnimationBuffer.reserve(maxMatrixCount);
}


/**
* アニメーションメッシュの描画用データをすべて削除
*
* フレームの最初に呼び出すこと。
*/
void GltfFileBuffer::ClearAnimationBuffer()
{
	tmpAnimationBuffer.clear();
}

/**
* アニメーションメッシュの描画用データを追加
*
* @param matBones SSBOに追加する姿勢行列の配列
+*
* @return matBones用に割り当てられたSSBOの範囲
*/
GltfFileRange GltfFileBuffer::AddAnimationMatrices(
	const AnimationMatrices& matBones)
{
	const GLintptr offset =
		static_cast<GLintptr>(tmpAnimationBuffer.size() * sizeof(mat4));
	tmpAnimationBuffer.insert(
		tmpAnimationBuffer.end(), matBones.begin(), matBones.end());

	// SSBOのオフセットアライメント条件を満たすために、256バイト境界(mat4の4個分)に配置する。
	// 256はOpenGL仕様で許される最大値。
	tmpAnimationBuffer.resize(((tmpAnimationBuffer.size() + 3) / 4) * 4);
	return { offset, matBones.size() * sizeof(mat4) };
}

/**
* アニメーションメッシュの描画用データをGPUメモリにコピー
*/
void GltfFileBuffer::UploadAnimationBuffer()
{
	if (tmpAnimationBuffer.empty()) {
		return; // 転送するデータがない場合は何もしない
	}
	animationBuffer->WaitSync();
	uint8_t* p = animationBuffer->GetMappedAddress();
	memcpy(p, tmpAnimationBuffer.data(), tmpAnimationBuffer.size() * sizeof(mat4));
	animationBuffer->SwapBuffers();
}

/**
* アニメーションメッシュの描画に使うSSBO領域を割り当てる
*
* @param bindingPoint バインディングポイント
* @param range        バインドする範囲
*/
void GltfFileBuffer::BindAnimationBuffer(
	GLuint bindingPoint, const GltfFileRange& range)
{
	if (range.size > 0) {
		animationBuffer->Bind(bindingPoint, range.offset, range.size);
	}
}

/**
* アニメーションメッシュの描画に使うSSBO領域の割り当てを解除する
*/
void GltfFileBuffer::UnbindAnimationBuffer(GLuint bindingPoint)
{
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingPoint, 0, 0, 0);
}


/**
* glTFファイルからGltfFileオブジェクトを作成する
*
* @param filename glTFファイル名
*
* @retval nullptr以外  filenameから作成したファイルオブジェクト
* @retval nullptr     読み込み失敗
*/
GltfFilePtr GltfFileBuffer::LoadGltf(const char* filename)
{
	// glTFファイルを読み込む
	std::vector<char> buf = ReadFile(filename);
	if (buf.empty()) {
		return nullptr;
	}
	buf.push_back('\0'); // テキスト終端を追加

	// フォルダ名を取り出す（filesystemライブラリのparent_path関数：フォルダ部分を取得する）
	const std::string foldername =
		std::filesystem::path(filename).parent_path().string() + '/';

	// JSONを解析
	auto p = Parse(buf.data(), foldername.c_str());
	if (!p) {
		LOG_ERROR("'%s'の読み込みに失敗しました", filename);
		return p;
	}

	// gltfファイル名を抽出する
	// '/' を区切り文字として文字列を分割
	std::string fname = filename;
	size_t lastSlash = fname.find_last_of("/");
	std::string gltffile = fname.substr(lastSlash + 1);	// 例：box.gltf

	// 作成したファイルを連想配列に追加
	p->name = gltffile;
	files.emplace(gltffile, p);

	// 読み込んだファイル名とメッシュ名をデバッグ情報として出力
	LOG("%sを読み込みました", filename);
	for (size_t i = 0; i < p->meshes.size(); ++i) {
		LOG(R"(  meshes[%d]="%s")", i, p->meshes[i].name.GetName().c_str());
	}

	// 読み込んだアニメーション名をデバッグ情報として出力
	for (size_t i = 0; i < p->animations.size(); ++i) {
		const std::string& name = p->animations[i]->name.GetName();
		if (name.size() <= 0) {
			LOG("  animations[%d]=<NO NAME>", i);
		}
		else {
			LOG(R"(  animations[%d]="%s")", i, name.c_str());
		}
	}

	return p;
}

/**
* glTFファイルの取得
*
* @param name glTfファイルの名前
*
* @return 名前がnameと一致するglTfファイル
*/
GltfFilePtr GltfFileBuffer::GetGltfFile(const char* name) const
{
	auto itr = files.find(MyFName(name));
	if (itr != files.end()) {
		//ファイル名に対応するglTfファイルを返す
		return itr->second;
	}
	return nullptr;
}

/**
* JSONテキストからGltfFileオブジェクトを作成する
*
* @param text       glTFのJSONテキスト
* @param foldername リソース読み込み用のフォルダ
*
* @retval nullptr以外  filenameから作成したファイルオブジェクト
* @retval nullptr     読み込み失敗
*/
GltfFilePtr GltfFileBuffer::Parse(const char* text, const char* foldername)
{
	// JSON解析
	json gltf = json::parse(text, nullptr, false);
	if (gltf.is_discarded()) {
		LOG_ERROR("JSONの解析に失敗しました");
		return nullptr;
	}

	// バイナリファイルを読み込む
	const GLsizeiptr prevBufferSize = curBufferSize;
	const json& buffers = gltf["buffers"];	// バイナリファイル名の配列を取得
	BinaryList binaryList(buffers.size());

	for (size_t i = 0; i < buffers.size(); ++i) {
		const auto uri = buffers[i].find("uri");
		if (uri == buffers[i].end()) {
			continue;
		}

		// ファイルを読み込む
		const std::string binPath = foldername + uri->get<std::string>();
		binaryList[i].bin = ReadFile(binPath.c_str());
		if (binaryList[i].bin.empty()) {
			curBufferSize = prevBufferSize; // 読み込んだデータをなかったことにする
			return nullptr; // バイナリファイルの読み込みに失敗
		}

		// バイナリデータをGPUメモリ(コンストラクタで作成したバッファオブジェクト)にコピー
		memcpy(pBuffer + curBufferSize, binaryList[i].bin.data(), binaryList[i].bin.size());

		// オフセットを更新
		binaryList[i].offset = curBufferSize;
		curBufferSize += binaryList[i].bin.size();
	}


	// ファイルオブジェクトを作成
	GltfFilePtr file = std::make_shared<GltfFile>();

	// ルートノードの姿勢制御行列を取得
	const auto nodes = gltf.find("nodes");
	if (nodes != gltf.end()) {
		// ノードが存在するかチェック
		if (!nodes->is_array() || nodes->empty()) {
			LOG_ERROR("glTFファイルの仕様違反：ノード数が0");
		}
		else {
			const json& rootNode = nodes->at(0);
			const auto matrix = rootNode.find("matrix");
			if (matrix != rootNode.end()) {
				if (matrix->size() >= 16) {
					for (int i = 0; i < 16; ++i) {
						file->matRoot[i / 4][i % 4] = matrix->at(i).get<float>();
					}
				}
			} // if matrix
		}
	} // if nodes


	// メッシュを作成
	const json& accessors = gltf["accessors"];
	const json& bufferViews = gltf["bufferViews"];
	const json& meshes = gltf["meshes"];
	file->meshes.reserve(meshes.size());
	for (const json& jsonMesh : meshes) {
		// メッシュ名を取得
		GltfMesh mesh;
		mesh.name = jsonMesh.value("name", "<default>");	// キーが存在しない場合は <default> を設定する

		// プリミティブを作成
		const json& primitives = jsonMesh["primitives"];
		mesh.primitives.reserve(primitives.size());
		for (const json& jsonPrim : primitives) {
			// VAOを作成
			GltfPrimitive prim;
			prim.vao = std::make_shared<VertexArrayObject>();

			// VAOをOpenGLコンテキストに割り当てる
			glBindVertexArray(*prim.vao);

			// VBOとIBOを、OpenGLコンテキストとVAOの両方に割り当てる（glTF用のVBOとIBOはバッファを共有している）
			glBindBuffer(GL_ARRAY_BUFFER, *buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer);


			// プリミティブのパラメータを取得
			{
				// プリミティブの種類
				prim.mode = jsonPrim.value("mode", GL_TRIANGLES);


				// インデックス数と型
				const int accessorId = jsonPrim["indices"].get<int>();
				const json& accessor = accessors[accessorId];

				prim.count = accessor["count"].get<int>();			// プリミティブの頂点数
				prim.type = accessor["componentType"].get<int>();	// インデックスデータの型


				// インデックスデータの開始位置
				const int bufferViewId = accessor["bufferView"].get<int>();
				const json& bufferView = bufferViews[bufferViewId];


				// バッファ内のインデックスデータの開始位置
				prim.indices = reinterpret_cast<const GLvoid*>(
					GetBinaryDataOffset(accessor, bufferView, binaryList));
			}


			const json& attributes = jsonPrim["attributes"];

			// 頂点アトリビュート(頂点座標)を取得
			const bool hasPosition = SetVertexAttribute(AttribIndex::position, "POSITION",
				attributes, accessors, bufferViews, binaryList);
			if (!hasPosition) {
				// パラメータがなかった場合はデフォルト頂点データを割り当てる
				SetDefaultAttribute(AttribIndex::position,
					3, offsetof(DefaultVertexData, position), *buffer);
			}

			// 頂点アトリビュート(テクスチャ座標)を取得
			const bool hasTexcoord0 = SetVertexAttribute(AttribIndex::texcoord0, "TEXCOORD_0",
				attributes, accessors, bufferViews, binaryList);
			if (!hasTexcoord0) {
				// パラメータがなかった場合はデフォルト頂点データを割り当てる
				SetDefaultAttribute(AttribIndex::texcoord0,
					2, offsetof(DefaultVertexData, texcoord0), *buffer);
			}

			// 頂点アトリビュート(法線)を取得
			const bool hasNormal = SetVertexAttribute(AttribIndex::normal, "NORMAL",
				attributes, accessors, bufferViews, binaryList);
			if (!hasNormal) {
				// パラメータがなかった場合はデフォルト頂点データを割り当てる
				SetDefaultAttribute(AttribIndex::normal,
					3, offsetof(DefaultVertexData, normal), *buffer);
			}

			// 頂点アトリビュート(タンジェント)を取得
			const bool hasTangent = SetVertexAttribute(AttribIndex::tangent, "TANGENT",
				attributes, accessors, bufferViews, binaryList);
			if (!hasTangent) {
				// パラメータがなかった場合はデフォルト頂点データを割り当てる
				SetDefaultAttribute(AttribIndex::tangent,
					4, offsetof(DefaultVertexData, tangent), *buffer);
			}

			// 頂点アトリビュート(ジョイント番号)を取得
			const auto hasJoints = SetVertexAttribute(AttribIndex::joints0, "JOINTS_0",
				attributes, accessors, bufferViews, binaryList);
			if (!hasJoints) {
				SetDefaultAttribute(AttribIndex::joints0,
					4, offsetof(DefaultVertexData, joints0), *buffer);
			}

			// 頂点アトリビュート(ジョイントウェイト)を取得
			const auto hasWeights = SetVertexAttribute(AttribIndex::weights0, "WEIGHTS_0",
				attributes, accessors, bufferViews, binaryList);
			if (!hasWeights) {
				SetDefaultAttribute(AttribIndex::weights0,
					4, offsetof(DefaultVertexData, weights0), *buffer);
			}

			// プリミティブが使用するマテリアル番号を取得
			prim.materialNo = jsonPrim.value("material", 0);

			// VAO, VBO, IBOのOpenGLコンテキストへの割り当てを解除
			glBindVertexArray(0);	/*※先にVAOを解除する（VBO,IBOを先に会場するとVAOへのバインドも解除されてしまうため）※*/
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// 作成したプリミティブを配列に追加
			mesh.primitives.push_back(prim);
		}

		// 作成したメッシュオブジェクトを配列に追加
		file->meshes.push_back(mesh);
	}


	// マテリアルを作成
	const auto materials = gltf.find("materials");
	if (materials != gltf.end()) {
		file->materials.reserve(materials->size());
		for (const json& material : *materials) {
			MaterialPtr m = std::make_shared<Material>();

			// 名前を設定
			m->name = material.value("name", std::string());

			// カラーを設定
			const auto pbr = material.find("pbrMetallicRoughness");	// カラー情報
			if (pbr != material.end()) {
				// マテリアルカラーを取得
				m->baseColor = vec4(1);
				const auto baseColorFactor = pbr->find("baseColorFactor");
				if (baseColorFactor != pbr->end()) {
					for (int i = 0; i < baseColorFactor->size(); ++i) {
						m->baseColor[i] = baseColorFactor->at(i).get<float>();
					}
				}

				// カラーテクスチャを読み込む
				m->texBaseColor = 
					LoadTexture("baseColorTexture", gltf, *pbr, foldername);

				// テクスチャが見つからなかった場合、White.tgaを設定しておく
				if (!m->texBaseColor) {
					m->texBaseColor = TextureManager::GetTexture("White.tga");
				}
			} // if pbr

			// 法線テクスチャを読み込む
			m->texNormal = 
				LoadTexture("normalTexture", gltf, material, foldername);

			// 発光カラーの取得
			m->emission = vec3(0);
			const auto emissiveFactor = material.find("emissiveFactor");
			if (emissiveFactor != material.end()) {
				for (int i = 0; i < emissiveFactor->size(); ++i) {
					m->emission[i] = emissiveFactor->at(i).get<float>();
				}
			}
			// 発光テクスチャを読み込む
			m->texEmission = 
				LoadTexture("emissiveTexture", gltf, material, foldername);

			// 作成したマテリアルを追加
			file->materials.push_back(m);
		}
	} // if materials

	// ノードを作成
	if (nodes != gltf.end()) {
		file->nodes.resize(nodes->size());
		for (size_t i = 0; i < nodes->size(); ++i) {
			const json& node = nodes->at(i);
			GltfNode& n = file->nodes[i];
			n.name = node.value("name", std::string());
			n.mesh = node.value("mesh", -1);
			n.skin = node.value("skin", -1);

			//LOG(n.name.GetName().c_str());	// TODO:ボーン名表示

			// 子ノードを取得し、子ノードに対して親ノードを設定
			const auto children = node.find("children");
			if (children != node.end()) {
				n.children.resize(children->size());
				for (int b = 0; b < children->size(); ++b) {
					const json& child = children->at(b);
					const int childId = child.get<int>();
					n.children[b] = &file->nodes[childId]; // 子ノードを追加
					n.children[b]->parent = &n; // 親ノードを設定
				}
			}
			// ローカル座標変換行列を計算
			n.matLocal = GetLocalMatrix(node);
		}

		// 親をたどってグローバル座標変換行列を計算する
		for (GltfNode& e : file->nodes) {
			e.matGlobal = e.matLocal;
			const GltfNode* parent = e.parent;
			while (parent) {
				e.matGlobal = parent->matLocal * e.matGlobal;
				parent = parent->parent;
			}
		}
	} // if nodes

	// シーンを作成
	const auto scenes = gltf.find("scenes");
	if (scenes != gltf.end()) {
		// 各シーンに表示するノード配列を取得
		file->scenes.resize(scenes->size());
		for (size_t a = 0; a < scenes->size(); ++a) {
			const json& scene = scenes->at(a);
			const auto nodes = scene.find("nodes");

			if (nodes == scene.end()) {
				continue;
			}
			GltfScene& s = file->scenes[a];
			s.nodes.resize(nodes->size());
			for (size_t b = 0; b < nodes->size(); ++b) {
				const int nodeId = nodes->at(b).get<int>();
				const GltfNode* n = &file->nodes[nodeId];
				s.nodes[b] = n;
				GetMeshNodeList(n, s.meshNodes);
			}
		}
	} // if scenes

	// スキンを取得する
	const auto skins = gltf.find("skins");
	if (skins != gltf.end()) {
		file->skins.resize(skins->size());
		for (size_t skinId = 0; skinId < skins->size(); ++skinId) {
			// スキン名を設定
			const json& skin = skins->at(skinId);
			GltfSkin& s = file->skins[skinId];
			s.name = skin.value("name", std::string());

			// 逆バインドポーズ行列（モデルのローカル座標系からボーンのローカル座標系に変換する行列）のアドレスを取得
			const mat4* inverseBindMatrices = nullptr;
			const auto ibm = skin.find("inverseBindMatrices");
			if (ibm != skin.end()) {
				const int ibmId = ibm->get<int>();
				inverseBindMatrices = static_cast<const mat4*>(
					GetBinaryDataAddress(accessors[ibmId], bufferViews, binaryList));
			}

			// 関節データを取得
			const json& joints = skin["joints"];
			s.joints.resize(joints.size());
			for (size_t jointId = 0; jointId < joints.size(); ++jointId) {
				auto& j = s.joints[jointId];
				j.nodeId = joints[jointId].get<int>();
				// 逆バインドポーズ行列が未指定の場合は単位行列を使う(glTF仕様 5.28.1)
				if (inverseBindMatrices) {
					j.matInverseBindPose = inverseBindMatrices[jointId];
				}
				else {
					j.matInverseBindPose = mat4(1);
				}
			}
		} // for skinId
	} // if skins

	const auto animations = gltf.find("animations");
	if (animations != gltf.end()) {
		file->animations.resize(animations->size());
		for (size_t animeId = 0; animeId < animations->size(); ++animeId) {
			// 名前を設定
			const json& animation = animations->at(animeId);
			GltfAnimationPtr a = std::make_shared<GltfAnimation>();
			a->name = animation.value("name", std::string());

			// チャネル配列の容量を予約
			// 一般的に、平行移動・回転・拡大縮小の3つはセットで指定するので、
			// 各チャネル配列のサイズは「総チャネル数 / 3」になる可能性が高い。
			// 安全のため、予測サイズが必ず1以上になるように1を足している。
			const json& channels = animation["channels"];
			// 3つのチャネル配列の予測サイズ（予測サイズが0になることを防ぐため1を足す）
			const size_t predictedSize = channels.size() / 3 + 1;
			a->translations.reserve(predictedSize);
			a->rotations.reserve(predictedSize);
			a->scales.reserve(predictedSize);

			// 全てのノードを「アニメーションなし」として登録
			a->staticNodes.resize(nodes->size());	// 処理の高速化のために使用
			for (int i = 0; i < nodes->size(); ++i) {
				a->staticNodes[i] = i;
			}

			// チャネル配列を設定
			const json& samplers = animation["samplers"];
			a->totalTime_s = 0;
			for (const json& e : channels) {
				// 時刻の配列を取得
				const json& sampler = samplers[e["sampler"].get<int>()];
				const json& inputAccessor = accessors[sampler["input"].get<int>()];
				const GLfloat* pTimes = static_cast<const GLfloat*>(
					GetBinaryDataAddress(inputAccessor, bufferViews, binaryList));	// TODO:安全のためにcomponentTypeがGL_FLOATであることをチェックする

				// 再生時間を計算
				a->totalTime_s =
					std::max(a->totalTime_s, inputAccessor["max"][0].get<float>());

				// 値の配列を取得
				const json& outputAccessor = accessors[sampler["output"].get<int>()];
				const void* pValues =
					GetBinaryDataAddress(outputAccessor, bufferViews, binaryList);

				// 補間方法を取得
				GltfInterpolation interp = GltfInterpolation::linear;
				const json& target = e["target"];
				const std::string& interpolation =
					target.value("interpolation", std::string());
				if (interpolation == "LINEAR") {
					interp = GltfInterpolation::linear;
				}
				else if (interpolation == "STEP") {
					interp = GltfInterpolation::step;
				}
				else if (interpolation == "CUBICSPLINE") {
					interp = GltfInterpolation::cubicSpline;
				}

				// 時刻と値の配列からチャネルを作成し、pathに対応する配列に追加
				const int inputCount = inputAccessor["count"].get<int>();
				const int targetNodeId = target["node"].get<int>();
				const std::string& path = target["path"].get<std::string>();

				if (path == "translation") {
					a->translations.push_back(MakeAnimationChannel<vec3>(
						pTimes, pValues, inputCount, targetNodeId, interp));
				}
				else if (path == "rotation") {
					a->rotations.push_back(MakeAnimationChannel<Quaternion>(
						pTimes, pValues, inputCount, targetNodeId, interp));
				}
				else if (path == "scale") {
					a->scales.push_back(MakeAnimationChannel<vec3>(
						pTimes, pValues, inputCount, targetNodeId, interp));
				}
				a->staticNodes[targetNodeId] = -1; // アニメーションあり
			}

			// 「アニメーションあり」をリストから削除する
			const auto itr = std::remove(
				a->staticNodes.begin(), a->staticNodes.end(), -1);
			a->staticNodes.erase(itr, a->staticNodes.end());
			a->staticNodes.shrink_to_fit();

			// アニメーションを設定
			file->animations[animeId] = a;
		}
	} // if animations

	return file; // 作成したファイルオブジェクトを返す
}

