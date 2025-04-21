/**
* @file Mesh.cpp
*/
#define _CRT_SECURE_NO_WARNINGS	/*sscanf�֐��ŃG���[�ɂȂ�Ȃ��悤�ɂ���*/
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
* MikkTSpace�p�̃��[�U�[�f�[�^
*/
struct UserData
{
	std::vector<Vertex>& vertices;
	std::vector<uint16_t>& indices;
};

/**
* MikkTSpace�̃C���^�[�t�F�C�X����
*/
class MikkTSpace
{
public:
	// �R���X�g���N�^
	MikkTSpace()
	{
		// ���f���̑��|���S������Ԃ��i���|���S�����̓C���f�b�N�X����1/3�j
		interface.m_getNumFaces = [](const SMikkTSpaceContext* pContext)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				return static_cast<int>(p->indices.size() / 3);
			};

		// �|���S���̒��_����Ԃ�
		interface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* pContext,
			int iFace)
			{
				return 3;	/*����͎O�p�`�����g��Ȃ����ߏ��3��Ԃ�*/
			};

		// ���_�̍��W��Ԃ��ifvPosOut�F���W�z��j
		interface.m_getPosition = [](const SMikkTSpaceContext* pContext,
			float fvPosOut[], int iFace, int iVert)
			{
				/*iFace�Ԗڂ̃|���S����iVert�Ԗڂ̒��_�̍��W���AfvPosOut�z���X,Y,Z�̏��ɑ������*/
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				fvPosOut[0] = p->vertices[index].position.x;
				fvPosOut[1] = p->vertices[index].position.y;
				fvPosOut[2] = p->vertices[index].position.z;
			};

		// ���_�̖@����Ԃ��ifvNormOut�F�@���z��j
		interface.m_getNormal = [](const SMikkTSpaceContext* pContext,
			float fvNormOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				fvNormOut[0] = p->vertices[index].normal.x;
				fvNormOut[1] = p->vertices[index].normal.y;
				fvNormOut[2] = p->vertices[index].normal.z;
			};

		// ���_�̃e�N�X�`�����W��Ԃ��ifvTexOut�F�e�N�X�`�����W ���񎟌��j
		interface.m_getTexCoord = [](const SMikkTSpaceContext* pContext,
			float fvTexcOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				fvTexcOut[0] = p->vertices[index].texcoord.x;
				fvTexcOut[1] = p->vertices[index].texcoord.y;
			};

		// �u�^���W�F���g�x�N�g���v�Ɓu�o�C�^���W�F���g�x�N�g���̌����v���󂯎��
		// 
		//@fvTangent(�G�t�u�C�E�^���W�F���g) : �^���W�F���g�x�N�g��
		//@fSign(�G�t�E�T�C��) : �o�C�^���W�F���g�x�N�g���̌���
		//@iFace(�A�C�E�t�F�C�X) : �f�[�^��ݒ肷��|���S���̔ԍ�
		//@iVert(�A�C�E�o�[�g) : �f�[�^��ݒ肷�钸�_�̔ԍ�
		interface.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext,
			const float fvTangent[], float fSign, int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				p->vertices[index].tangent =
					vec4(fvTangent[0], fvTangent[1], fvTangent[2], fSign);
			};

		// �g��Ȃ��̂�nullptr��ݒ�
		interface.m_setTSpace = nullptr;
	}

	// �f�X�g���N�^
	~MikkTSpace() = default;

	// �����o�ϐ�
	SMikkTSpaceInterface interface;
};


/**
* MTL�t�@�C����ǂݍ���
*
* @param foldername OBJ�t�@�C���̂���t�H���_��
* @param filename   MTL�t�@�C����
*
* @return MTL�t�@�C���Ɋ܂܂��}�e���A���̔z��
*/
std::vector<MaterialPtr> MeshBuffer::LoadMTL(
	const std::string& foldername, const char* filename)
{
	// MTL�t�@�C�����J��
	const std::string fullpath = foldername + filename;
	std::ifstream file(fullpath);
	if (!file) {
		LOG_ERROR("%s���J���܂���", fullpath.c_str());
		return {};
	}

	// MTL�t�@�C������͂���
	std::vector<MaterialPtr> materials;
	MaterialPtr pMaterial;
	vec3 specularColor = vec3(1); // �X�y�L�����F
	float specularPower = 12;     // �X�y�L�����W��
	while (!file.eof()) {
		std::string line;
		std::getline(file, line);

		// �}�e���A����`�̓ǂݎ������݂�
		char name[1000] = { 0 };
		if (sscanf(line.data(), " newmtl %[^\n]s", name) == 1) {
			if (pMaterial) {
				// �X�y�L�����p�����[�^���烉�t�l�X���v�Z
				// NOTE: �V�F�[�_�Ŗ@�����z�����v�Z����Ƃ��ɕ��ꂪ0�ɂȂ�Ȃ��悤�ɂ��邽�߁A���t�l�X��0���傫���Ȃ��Ă͂Ȃ�Ȃ�
				specularPower *=
					std::max(std::max(specularColor.x, specularColor.y), specularColor.z);
				pMaterial->roughness = std::clamp(1 - log2(specularPower) / 12, 0.0001f, 1.0f);

				// �X�y�L�����p�����[�^�������l�ɖ߂�
				specularColor = vec3(1);
				specularPower = 12;

				// �e�N�X�`�����ݒ肳��Ă��Ȃ��}�e���A���ꍇ�AWhite.tga��ݒ肵�Ă���
				if (!pMaterial->texBaseColor) {
					pMaterial->texBaseColor = TextureManager::GetTexture("White.tga");
				}
			}
			pMaterial = std::make_shared<Material>();
			pMaterial->name = name;
			materials.push_back(pMaterial);
			continue;
		}

		// �}�e���A������`����Ă��Ȃ��ꍇ�͍s�𖳎�����
		if (!pMaterial) {
			continue;
		}

		// ��{�F�̓ǂݎ������݂�
		if (sscanf(line.data(), " Kd %f %f %f",
			&pMaterial->baseColor.x, &pMaterial->baseColor.y, 
			&pMaterial->baseColor.z) == 3) {
			continue;
		}

		// �s�����x�̓ǂݎ������݂�
		if (sscanf(line.data(), " d %f", &pMaterial->baseColor.w) == 1) {
			continue;
		}

		// ��{�F�e�N�X�`�����̓ǂݎ������݂�
		char textureName[1000] = { 0 };
		if (sscanf(line.data(), " map_Kd %[^\n]s", &textureName) == 1) {
			const std::string filename = foldername + textureName;
			if (std::filesystem::exists(filename)) {
				pMaterial->texBaseColor =
					TextureManager::GetTexture(filename.c_str());
			}
			else {
				LOG_WARNING("%s���J���܂���", filename.c_str());
			}
			continue;
		}

		// �����F�̓ǂݎ������݂�
		if (sscanf(line.data(), " Ke %f %f %f",
			&pMaterial->emission.x, &pMaterial->emission.y,
			&pMaterial->emission.z) == 3) {
			continue;
		}

		// �����F�e�N�X�`�����̓ǂݎ������݂�
		if (sscanf(line.data(), " map_Ke %[^\n]s", &textureName) == 1) {
			const std::string filename = foldername + textureName;
			if (std::filesystem::exists(filename)) {
				pMaterial->texEmission =
					TextureManager::GetTexture(filename.c_str());
			}
			else {
				LOG_WARNING("%s���J���܂���", filename.c_str());
			}
			continue;
		}

		// �@���e�N�X�`�����̓ǂݎ������݂�
		if (sscanf(line.data(), " map_%*[Bb]ump %[^\n]s", &textureName) == 1) {
			const std::string filename = foldername + textureName;
			if (std::filesystem::exists(filename)) {
				pMaterial->texNormal =
					TextureManager::GetTexture(filename.c_str());
			}
			else {
				LOG_WARNING("%s���J���܂���", filename.c_str());
			}
			continue;
		}

		// �X�y�L�����F�̓ǂݎ������݂�
		if (sscanf(line.data(), " Ks %f %f %f",
			&specularColor.x, &specularColor.y, &specularColor.z) == 3) {
			continue;
		}

		// �X�y�L�����W���̓ǂݎ������݂�
		if (sscanf(line.data(), " Ns %f", &specularPower) == 1) {
			continue;
		}

		// ���^���b�N�̓ǂݎ������݂�
		if (sscanf(line.data(), " Pm %f", &pMaterial->metallic) == 1) {
			continue;
		}
	}

	// �Ō�̃}�e���A���̃X�y�L�����p�����[�^��ݒ�
	if (pMaterial) {
		// �X�y�L�����p�����[�^���烉�t�l�X���v�Z
		specularPower *= std::max(std::max(specularColor.x, specularColor.y), specularColor.z);
		pMaterial->roughness = std::clamp(1 - log2(specularPower) / 12, 0.000000001f, 1.0f);

		// �e�N�X�`�����ݒ肳��Ă��Ȃ��}�e���A���ꍇ�AWhite.tga��ݒ肵�Ă���
		if (!pMaterial->texBaseColor) {
			pMaterial->texBaseColor = TextureManager::GetTexture("White.tga");
		}
	}

	// �ǂݍ��񂾃}�e���A���̔z���Ԃ�
	return materials;
}

/**
* �R���X�g���N�^
*/
MeshBuffer::MeshBuffer(size_t bufferSize)
	:primitiveBuffer(bufferSize)
{
	// �^���W�F���g��Ԍv�Z�p�̃I�u�W�F�N�g���쐬����
	mikkTSpace = std::make_shared<MikkTSpace>();
	if (!mikkTSpace) {
		LOG_ERROR("MikkTSpace�̍쐬�Ɏ��s");
	}

	// �X�^�e�B�b�N���b�V���̗e�ʂ�\��
	meshes.reserve(100);
}

/**
* OBJ�t�@�C����ǂݍ���
*
* @param filename OBJ�t�@�C����
*
* @return filename����쐬�������b�V��
*/
StaticMeshPtr MeshBuffer::LoadOBJ(const char* filename)
{
	// �ȑO�ɓǂݍ��񂾃t�@�C���Ȃ�A�쐬�ς݂̃��b�V����Ԃ�
	{
		auto itr = meshes.find(filename);
		if (itr != meshes.end()) {	/*�L�[�ƈʒu����f�[�^������ꍇ*/
			return itr->second;
		}
	}/*itr�ϐ����㑱�̃v���O�������猩���Ȃ����邽��*/

	// OBJ�t�@�C�����J��
	std::ifstream file(filename);
	if (!file) {
		LOG_ERROR("%s���J���܂���", filename);
		return nullptr;
	}

	// �t�H���_�����擾����
	std::string foldername(filename);
	{
		const size_t p = foldername.find_last_of("����/");
		if (p != std::string::npos) {
			foldername.resize(p + 1);
		}
	}

	// OBJ�t�@�C������͂��āA���_�f�[�^�ƃC���f�b�N�X�f�[�^��ǂݍ���
	std::vector<vec3> positions;
	std::vector<vec2> texcoords;
	std::vector<vec3> normals;
	struct IndexSet { int v, vt, vn; };
	std::vector<IndexSet> faceIndexSet;	/*�C���f�b�N�X�f�[�^�̑g*/

	positions.reserve(20'000);
	texcoords.reserve(20'000);
	normals.reserve(20'000);
	faceIndexSet.reserve(20'000 * 3);	/*���_���Ɠ������̎O�p�`�������ɂ�3�{����K�v������*/

	// �}�e���A��
	std::vector<MaterialPtr> materials;
	materials.reserve(100);

	// �}�e���A���̎g�p�͈�
	struct UseMaterial {
		std::string name;   // �}�e���A����
		size_t startOffset; // ���蓖�Ĕ͈͂̐擪�ʒu
	};
	std::vector<UseMaterial> usemtls;
	usemtls.reserve(100);

	// ���f�[�^��ǉ�(�}�e���A���w�肪�Ȃ��t�@�C���΍�)
	usemtls.push_back({ std::string(), 0 });


	while (!file.eof()) {
		std::string line;
		std::getline(file, line);	/*��s���ǂݎ��*/
		const char* p = line.c_str();	/*sscanf�ɓn�����߂�const char*�ɕϊ�*/

		// ���_���W�̓ǂݎ������݂�
		vec3 v;
		if (sscanf(p, " v %f %f %f", &v.x, &v.y, &v.z) == 3) {	/*�����w�蕶����̐擪�Ɂu�󔒁v�����i�s���̋󔒂�^�u���΂��Ă���邽�߁j*/
			positions.push_back(v);								/*%n�̌��ʁu%n�������܂łɓǂݎ�����������������Ɋi�[����v*/
			continue;
		}

		// �e�N�X�`�����W�̓ǂݎ������݂�
		vec2 vt;
		if (sscanf(p, " vt %f %f", &vt.x, &vt.y) == 2) {	/*�����w�蕶����̐擪�Ɂu�󔒁v�����*/
			texcoords.push_back(vt);
			continue;
		}

		// �@���̓ǂݎ������݂�
		vec3 vn;
		if (sscanf(p, " vn %f %f %f", &vn.x, &vn.y, &vn.z) == 3) {
			normals.push_back(vn);
			continue;
		}

		// �C���f�b�N�X�f�[�^�̓ǂݎ������݂�
		IndexSet f0, f1, f2;
		int readByte;

		// ���_���W+�e�N�X�`�����W+�@��
		if (sscanf(p, " f %u/%u/%u %u/%u/%u%n",
			&f0.v, &f0.vt, &f0.vn, &f1.v, &f1.vt, &f1.vn, &readByte) == 6) {
			p += readByte; // �ǂݎ��ʒu���X�V�i�ǂݎ���������������Z�j
			for (;;) {
				if (sscanf(p, " %u/%u/%u%n", &f2.v, &f2.vt, &f2.vn, &readByte) != 3) {
					break;
				}
				p += readByte; // �ǂݎ��ʒu���X�V�i�ǂݎ���������������Z�j
				faceIndexSet.push_back(f0);
				faceIndexSet.push_back(f1);
				faceIndexSet.push_back(f2);
				f1 = f2; // ���̎O�p�`�̂��߂Ƀf�[�^���ړ�
			}
			continue;
		}

		// ���_���W+�e�N�X�`�����W
		if (sscanf(p, " f %u/%u %u/%u%n",	/*(�@��������ꍇ��/�̈ʒu���قȂ邽�߁Aif�͐��藧���Ȃ��Ȃ�)*/
			&f0.v, &f0.vt, &f1.v, &f1.vt, &readByte) == 4) {
			f0.vn = f1.vn = 0; // �@���Ȃ�
			p += readByte; // �ǂݎ��ʒu���X�V�i�ǂݎ���������������Z�j
			for (;;) {
				if (sscanf(p, " %u/%u%n", &f2.v, &f2.vt, &readByte) != 2) {
					break;
				}
				f2.vn = 0; // �@���Ȃ�
				p += readByte; // �ǂݎ��ʒu���X�V�i�ǂݎ���������������Z�j
				faceIndexSet.push_back(f0);
				faceIndexSet.push_back(f1);
				faceIndexSet.push_back(f2);
				f1 = f2; // ���̎O�p�`�̂��߂Ƀf�[�^���ړ�	

				/*f0����Ƃ���O�p�`���擾���Ă���*/
			}
			continue;
		}

		// MTL�t�@�C���̓ǂݎ������݂�
		char mtlFilename[1000];
		if (sscanf(line.data(), " mtllib %999s", mtlFilename) == 1) {
			const auto tmp = LoadMTL(foldername, mtlFilename);
			materials.insert(materials.end(), tmp.begin(), tmp.end());	/*�}�e���A���z��ɒǉ�����*/
			continue;
		}

		// �g�p�}�e���A�����̓ǂݎ������݂�
		char mtlName[1000];
		if (sscanf(line.data(), " usemtl %999s", mtlName) == 1) {
			usemtls.push_back({ mtlName, faceIndexSet.size() });
			continue;
		}

	} // while eof

	// �����ɔԕ���ǉ�	/*�u���̃}�e���A��������v�Ƃ������ƑO��Ńv���O������������*/
	usemtls.push_back({ std::string(), faceIndexSet.size() });


	// OBJ�t�@�C����f�\����OpenGL�̒��_�C���f�b�N�X�z��̑Ή��\
	/* v, vn, vt�����ꂼ��20bit�ŏ\���Ȃ��߁u20bit�~3 = 60bit�v�Ȃ̂� uint64_t
	   �A�z�z��̒l�͒��_�C���f�b�N�X�Ȃ̂�uint16_t */
	std::unordered_map<uint64_t, uint16_t> indexMap;
	indexMap.reserve(10'000);


	std::vector<Vertex> vertices;
	vertices.reserve(faceIndexSet.size());
	std::vector<uint16_t> indices;
	indices.reserve(faceIndexSet.size());

	// �ǂݍ��񂾃f�[�^���AOpenGL�Ŏg����f�[�^�ɕϊ�
	for (const auto& e : faceIndexSet) {

		// f�\���̒l��64�r�b�g�́u�L�[�v�ɕϊ�
		const uint64_t key = static_cast<uint64_t>(e.v) +
			(static_cast<uint64_t>(e.vt) << 20) + (static_cast<uint64_t>(e.vn) << 40);

		// �Ή��\����L�[�Ɉ�v����f�[�^������
		const auto itr = indexMap.find(key);
		if (itr != indexMap.end()) {
			// �Ή��\�ɂ���̂Ŋ����̒��_�C���f�b�N�X���g��
			indices.push_back(itr->second);
		}
		else {
			// �Ή��\�ɂȂ��̂ŐV�������_�f�[�^���쐬���A���_�z��ɒǉ�
			Vertex v;
			v.position = positions[e.v - 1];
			v.texcoord = texcoords[e.vt - 1];
			/*-1���Ă���̂�OBJ�t�@�C���̃C���f�b�N�X��1����n�܂�̂ɑ΂��AC++����̓Y����0���炾����*/

			 // �@�����ݒ肳��Ă��Ȃ��ꍇ��0��ݒ�(���ƂŌv�Z)
			if (e.vn == 0) {
				v.normal = { 0, 0, 0 };
			}
			else {
				v.normal = normals[e.vn - 1];
			}

			vertices.push_back(v);

			// �V�������_�f�[�^�̃C���f�b�N�X���A���_�C���f�b�N�X�z��ɒǉ�
			const uint16_t index = static_cast<uint16_t>(vertices.size() - 1);
			indices.push_back(index);

			// �L�[�ƒ��_�C���f�b�N�X�̃y�A��Ή��\�ɒǉ�
			indexMap.emplace(key, index);
		}
	}

	// �^���W�F���g�x�N�g�����v�Z
	if (mikkTSpace) {
		// MikkTSpace���C�u�������g���ă^���W�F���g���v�Z
		UserData userData = { vertices, indices };
		SMikkTSpaceContext context = { &mikkTSpace->interface, &userData };
		genTangSpaceDefault(&context);
	}
	else {
		// �蓮�Ń^���W�F���g���v�Z
		for (size_t i = 0; i < indices.size(); ++i) {
			// ���_���W�ƃe�N�X�`�����W�̕Ӄx�N�g�����v�Z	

			//�|���S�����\�����钸�_�C���f�b�N�X�̃I�t�Z�b�g
			const size_t offset = (i / 3) * 3;
			const int i0 = indices[(i + 0) % 3 + offset];
			const int i1 = indices[(i + 1) % 3 + offset];
			const int i2 = indices[(i + 2) % 3 + offset];

			Vertex& v0 = vertices[i0];
			Vertex& v1 = vertices[i1];
			Vertex& v2 = vertices[i2];
			vec3 e1 = v1.position - v0.position; // ���W�x�N�g��
			vec3 e2 = v2.position - v0.position; // ���W�x�N�g��
			vec2 uv1 = v1.texcoord - v0.texcoord; // �e�N�X�`�����W�x�N�g��
			vec2 uv2 = v2.texcoord - v0.texcoord; // �e�N�X�`�����W�x�N�g��

			// �e�N�X�`���̖ʐς��ق�0�̏ꍇ�͖���
			float uvArea = uv1.x * uv2.y - uv1.y * uv2.x; // uv�x�N�g���ɂ�镽�s�l�ӌ`�̖ʐ�
			if (abs(uvArea) < 0x1p-20) { // �\�Z�i���������_�����e����(C++17)
				continue;
			}

			// �e�N�X�`�����������ɓ\���Ă���(�ʐς�����)�ꍇ�A�o�C�^���W�F���g�x�N�g���̌������t�ɂ���
			if (uvArea >= 0) {
				v0.tangent.w = 1; // �\����
			}
			else {
				v0.tangent.w = -1;// ������
			}

			// �^���W�F���g���ʂƕ��s�ȃx�N�g���ɂ��邽�߁A�@�������̐���������
			e1 -= v0.normal * dot(e1, v0.normal);
			e2 -= v0.normal * dot(e2, v0.normal);

			// �^���W�F���g�x�N�g�����v�Z
			vec3 tangent = normalize((uv2.y * e1 - uv1.y * e2) * v0.tangent.w);

			// �Ӄx�N�g���̂Ȃ��p���d�v�x�Ƃ��ă^���W�F���g������
			float angle = acos(dot(e1, e2) / (length(e1) * length(e2)));
			v0.tangent += vec4(tangent * angle, 0);

			// �^���W�F���g�x�N�g���𐳋K���i���ϒl���o���Ă���j
			for (auto& e : vertices) {
				vec3 t = normalize(vec3(e.tangent.x, e.tangent.y, e.tangent.z));
				e.tangent = vec4(t.x, t.y, t.z, e.tangent.w);
			}
		}
	} // mikkTSpace

	// �ݒ肳��Ă��Ȃ��@����₤	
	/*FillMissingNormals(vertices.data(), vertices.size(),
		indices.data(), indices.size());*/

		// �ϊ������f�[�^���o�b�t�@�ɒǉ�
	primitiveBuffer.AddPrimitive(
		vertices.data(), vertices.size() * sizeof(Vertex),
		indices.data(), indices.size() * sizeof(uint16_t));

	// ���b�V�����쐬
	const auto& pMesh = std::make_shared<StaticMesh>();

	// �f�[�^�̈ʒu���擾
	const void* indexOffset = primitiveBuffer.GetPrimitiveBack().indices;
	const GLint baseVertex = primitiveBuffer.GetPrimitiveBack().baseVertex;


	// �}�e���A���ɑΉ������`��p�����[�^���쐬
	// ���f�[�^�Ɣԕ��ȊO�̃}�e���A��������ꍇ�A���f�[�^���΂�
	size_t i = 0;
	if (usemtls.size() > 2) {
		i = 1; // ���f�[�^�Ɣԕ��ȊO�̃}�e���A��������ꍇ�A���f�[�^���΂�
	}

	//�Ō�̃}�e���A���ɂ́u���̃}�e���A���v���Ȃ��̂ŗ�O�������s��
	for (; i < usemtls.size() - 1; ++i) {
		const UseMaterial& cur = usemtls[i]; // �g�p���̃}�e���A��
		const UseMaterial& next = usemtls[i + 1]; // ���̃}�e���A��
		if (next.startOffset == cur.startOffset) {
			continue; // �C���f�b�N�X�f�[�^���Ȃ��ꍇ�͔�΂�
		}

		// �`��p�����[�^���쐬
		Primitive prim;
		prim.mode = GL_TRIANGLES;
		prim.count = static_cast<GLsizei>(next.startOffset - cur.startOffset);
		prim.indices = indexOffset;
		prim.baseVertex = baseVertex;
		prim.materialNo = 0; // �f�t�H���g�l��ݒ�
		for (int i = 0; i < materials.size(); ++i) {
			if (materials[i]->name == cur.name) {
				prim.materialNo = i; // ���O�̈�v����}�e���A����ݒ�
				break;
			}
		}
		pMesh->primitives.push_back(prim);

		// �C���f�b�N�X�I�t�Z�b�g��ύX
		indexOffset = reinterpret_cast<void*>(
			reinterpret_cast<size_t>(indexOffset) + sizeof(uint16_t) * prim.count);
	}

	// �}�e���A���z�񂪋�̏ꍇ�A�f�t�H���g�}�e���A����ǉ�
	if (materials.empty()) {
		pMesh->materials.push_back(std::make_shared<Material>());
	}
	else {
		pMesh->materials.assign(materials.begin(), materials.end());	/*�}�e���A�������b�V���ɃR�s�[*/
	}

	// obj�t�@�C�����𒊏o����
	// '/' ����؂蕶���Ƃ��ĕ�����𕪊�
	std::string fname = filename;
	size_t lastSlash = fname.find_last_of("/");
	std::string objfile = fname.substr(lastSlash + 1);	// ��Fbox.obj

	pMesh->name = objfile;
	meshes.emplace(objfile, pMesh);	/*�f�[�^�ɒǉ�*/

	LOG("%s��ǂݍ��݂܂���(���_��=%d, �C���f�b�N�X��=%d)",
		filename, vertices.size(), indices.size());


	// �쐬�������b�V����Ԃ�
	return pMesh;
}

/**
* �S�Ă̒��_�f�[�^���폜
*/
void MeshBuffer::Clear()
{
	meshes.clear();
	primitiveBuffer.Clear();
}


// �����Ă���@����₤
void FillMissingNormals(
	Vertex* vertices, size_t vertexCount,
	const uint16_t* indices, size_t indexCount)
{
	// �@�����ݒ肳��Ă��Ȃ����_��������
	std::vector<bool> missingNormals(vertexCount, false);
	for (int i = 0; i < vertexCount; ++i) {
		// �@���̒�����0�̏ꍇ���u�ݒ肳��Ă��Ȃ��v�Ƃ݂Ȃ�
		const vec3& n = vertices[i].normal;
		if (n.x == 0 && n.y == 0 && n.z == 0) {
			missingNormals[i] = true;	/*�ݒ肳��Ă��Ȃ�*/
		}
	}

	// �@�����v�Z�i3�̒��_�x�N�g���̕��ς���@���x�N�g�������߂�j
	for (int i = 0; i < indexCount; i += 3) {
		// �ʂ��\������2��a, b�����߂�
		const int i0 = indices[i + 0];
		const int i1 = indices[i + 1];
		const int i2 = indices[i + 2];

		// �����ꂩ�̒��_���̖@�����ݒ肳��Ă��Ȃ�������v�Z����
		if (missingNormals[i0] ||
			missingNormals[i1] ||
			missingNormals[i2])
		{
			const vec3& v0 = vertices[i0].position;	/*���ꂼ��̒��_���W�𓾂�*/
			const vec3& v1 = vertices[i1].position;
			const vec3& v2 = vertices[i2].position;
			const vec3 a = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };	/*v0����v1�Ɍ������x�N�g��*/
			const vec3 b = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };	/*v0����v2�Ɍ������x�N�g��*/

			// �O�ςɂ����a��b�ɐ����ȃx�N�g��(�@��)�����߂�
			const float cx = a.y * b.z - a.z * b.y;
			const float cy = a.z * b.x - a.x * b.z;
			const float cz = a.x * b.y - a.y * b.x;

			// �@���𐳋K�����ĒP�ʃx�N�g���ɂ���	/*�e�x�N�g���̐������x�N�g���̒����Ŋ���*/
			const float l = sqrt(cx * cx + cy * cy + cz * cz);
			const vec3 normal = { cx / l, cy / l, cz / l };

			// �@�����ݒ肳��Ă��Ȃ����_�ɂ����@�������Z
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

	// �S���̌v�Z���I���������߂Ė@���𐳋K��
	for (int i = 0; i < vertexCount; ++i) {
		if (missingNormals[i]) {
			vec3& n = vertices[i].normal;
			const float l = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
			n = { n.x / l, n.y / l, n.z / l };
		}
	} // for i
}

// �X�^�e�B�b�N���b�V�����擾
StaticMeshPtr MeshBuffer::GetStaticMesh(const char* name) const
{
	const auto& itr = meshes.find(MyFName(name));
	if (itr != meshes.end()) {
		//�t�@�C�����ɑΉ�����X�^�e�B�b�N���b�V����Ԃ�
		return itr->second;
	}

	LOG_ERROR("%s��������܂���ł���", name);
	return nullptr;
}


VertexArrayObjectPtr MeshBuffer::GetVAO() const
{
	return primitiveBuffer.GetVAO();
}

/**
* �����̕`��p�����[�^�ƃe�N�X�`������V�����X�^�e�B�b�N���b�V�����쐬����
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
* XY���ʂ̃v���[�g���b�V�����쐬����
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

	// �@����ݒ肷�邽�߂ɐ}�`�f�[�^�̃R�s�[�����
	auto pVertex = static_cast<const Vertex*>(vertexData);
	auto pIndex = static_cast<const uint16_t*>(indexData);
	std::vector<Vertex> v(pVertex, pVertex + sizeof(vertexData) / sizeof(Vertex));

	// �R�s�[�����}�`�f�[�^�ɖ@����ݒ�
	for (auto& e : v) {
		e.normal = { 0, 0, 1 };
		e.tangent = { -1, 0, 0, 1 };
	}
	//FillMissingNormals(v.data(), v.size(), pIndex, sizeof(indexData) / sizeof(uint16_t));

	// �o�b�t�@�ɒǉ�
	primitiveBuffer.AddPrimitive(vertexData, sizeof(vertexData), indexData, sizeof(indexData));

	// ���O��AddPrimitive�ō쐬�����`��p�����[�^���擾
	const Primitive& drawPrim =
		primitiveBuffer.GetPrimitive(primitiveBuffer.GetPrimitivesCount() - 1);

	return CreateStaticMesh(name, drawPrim, nullptr);
}
