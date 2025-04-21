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

	// �R���X�g���N�^�A�f�X�g���N�^���Ăׂ�悤�ɂ��邽�߂̕⏕�N���X
	struct TexHelper : public Texture {
		TexHelper(const char* p, Usage u = Usage::for3D) : 
			Texture(p, u) {}
		TexHelper(const char* p, int w, int h, GLenum f, int levels = 1) :
			Texture(p, w, h, f, GL_CLAMP_TO_EDGE, levels) {}
	};

public:

	/**
	* ��̃e�N�X�`�����쐬����i�L���b�V���ɂ͓o�^����Ȃ��j
	*
	* @param name      �e�N�X�`�����ʗp�̖��O
	* @param width     �e�N�X�`���̕�(�s�N�Z����)
	* @param heightOffset    �e�N�X�`���̍���(�s�N�Z����)
	* @param gpuFormat �f�[�^�`��
	* @param levels    �~�b�v�}�b�v�e�N�X�`���̃��x����
	*/
	static TexturePtr CreateTexture(const char* name, int width, int height,
		GLenum gpuFormat, int levels = 1)
	{
		return std::make_shared<TexHelper>(name, width, height, gpuFormat, levels);
	}

	/**
	* �e�N�X�`���̎擾
	*
	* @param name  �e�N�X�`���t�@�C����
	* @param usage ���p�ړI
	*
	* @return ���O��name�ƈ�v����e�N�X�`��
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
	// �L���b�V������e�N�X�`���̎擾
	TexturePtr GetTex(const MyFName& fname, Usage usage = Usage::for3D) {
		// �L���b�V���ɂ���΁A�L���V�����ꂽ�e�N�X�`����Ԃ�
		const auto& itr = textureCache.find(fname);
		if (itr != textureCache.end()) {
			return itr->second; // �L���b�V�����ꂽ�e�N�X�`����Ԃ�
		}

		// �L���b�V���ɂȂ���΁A�e�N�X�`�����쐬���ăL���b�V���ɓo�^
		const auto& tex = 
			std::make_shared<TexHelper>(fname.GetName().c_str(), usage);
		textureCache.emplace(fname, tex);

		return tex; // �쐬�����e�N�X�`����Ԃ�
	}
};

#endif // TEXTUREMANAGER_H_INCLUDED