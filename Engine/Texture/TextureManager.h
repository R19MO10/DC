/**
* @file TextureManager.h
*/
#ifndef TEXTUREMANAGER_H_INCLUDED
#define TEXTUREMANAGER_H_INCLUDED
#include "../../Utility/Singleton.h"
#include "Texture.h"
#include <string>
#include <unordered_map>

class TextureManager : public Singleton<TextureManager>
{
private:
	std::unordered_map<MyFName, TexturePtr> textureCache;

	// コンストラクタ、デストラクタを呼べるようにするための補助クラス
	struct TexHelper : public Texture {
		TexHelper(const char* p, Usage u = Usage::for3D) : 
			Texture(p, u) {}
		TexHelper(const char* p, int w, int h, GLenum f, int levels = 1) :
			Texture(p, w, h, f, GL_CLAMP_TO_EDGE, levels) {}
	};

public:

	/**
	* 空のテクスチャを作成する（キャッシュには登録されない）
	*
	* @param name      テクスチャ識別用の名前
	* @param width     テクスチャの幅(ピクセル数)
	* @param heightOffset    テクスチャの高さ(ピクセル数)
	* @param gpuFormat データ形式
	* @param levels    ミップマップテクスチャのレベル数
	*/
	static TexturePtr CreateTexture(const char* name, int width, int height,
		GLenum gpuFormat, int levels = 1)
	{
		return std::make_shared<TexHelper>(name, width, height, gpuFormat, levels);
	}

	/**
	* テクスチャの取得
	*
	* @param name  テクスチャファイル名
	* @param usage 利用目的
	*
	* @return 名前がnameと一致するテクスチャ
	*/
	static TexturePtr GetTexture(const char* name, Usage usage = Usage::for3D)
	{
		return GetInstance().GetTex(MyFName(name), usage);
	}
	static TexturePtr GetTexture(const std::string& name, Usage usage = Usage::for3D)
	{
		return GetInstance().GetTex(MyFName(name), usage);
	}

private:
	// キャッシュからテクスチャの取得
	TexturePtr GetTex(const MyFName& fname, Usage usage = Usage::for3D) {
		// キャッシュにあれば、キャシュされたテクスチャを返す
		const auto& itr = textureCache.find(fname);
		if (itr != textureCache.end()) {
			return itr->second; // キャッシュされたテクスチャを返す
		}

		// キャッシュになければ、テクスチャを作成してキャッシュに登録
		const auto& tex = 
			std::make_shared<TexHelper>(fname.GetName().c_str(), usage);
		textureCache.emplace(fname, tex);

		return tex; // 作成したテクスチャを返す
	}
};

#endif // TEXTUREMANAGER_H_INCLUDED