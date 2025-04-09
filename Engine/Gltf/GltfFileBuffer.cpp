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

//�u�s�v�ɂȂ����f�[�^���J�����鏈���v�͎������܂���B�K�v�ɉ����Ă݂Ȃ��񎩐g���������Ă��������B
using json = nlohmann::json;


namespace {
	/**
	* �o�C�i���f�[�^
	*/
	struct BinaryData
	{
		GLsizeiptr offset;     // GPU��������̃f�[�^�J�n�I�t�Z�b�g
		std::vector<char> bin; // CPU�������ɓǂݍ��񂾃f�[�^
	};
	using BinaryList = std::vector<BinaryData>;
	/**
	* �t�@�C����ǂݍ���
	*
	* @param filename �ǂݍ��ރt�@�C����
	*
	* @return �ǂݍ��񂾃f�[�^�z��
	*/
	std::vector<char> ReadFile(const char* filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file) {
			LOG_WARNING("%s���J���܂���", filename);
			return {};
		}
		std::vector<char> buf(std::filesystem::file_size(filename));
		file.read(buf.data(), buf.size());
		return buf;
	}


	/**
	* ���_�A�g���r���[�g�ԍ�
	* VAO���Ǘ����Ă��钸�_�A�g���r���[�g�z��̂����u���Ԗڂ̒��_�A�g���r���[�g�𑀍삷��̂��v���w�肷��
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
	* �K�v�Ȓ��_�f�[�^�v�f���v���~�e�B�u�ɑ��݂��Ȃ��ꍇ�Ɏg���f�[�^
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
	* �����̐����擾����
	*
	* @param accessor �p�����[�^�����A�N�Z�T
	*
	* @return �����̐�
	*/
	int GetComponentCount(const json& accessor)
	{
		// �^���Ɛ������̑Ή��\
		static const struct {
			const char* type;   // �^�̖��O
			int componentCount; // ������
		} componentCountList[] = {
			{ "SCALAR", 1 },
			{ "VEC2", 2 }, { "VEC3", 3 }, { "VEC4", 4 },
			{ "MAT2", 4 }, { "MAT3", 9 }, { "MAT4", 16 },
		};
		// �Ή��\���琬�������擾
		const std::string& type = accessor["type"].get<std::string>();
		for (const auto& e : componentCountList) {
			if (type == e.type) {
				return e.componentCount;
			}
		}
		LOG_WARNING("�s���Ȍ^`%s`���w�肳��Ă��܂�", type.c_str());
		return 1; // glTF�̎d�l�ǂ���Ȃ�A�����ɗ��邱�Ƃ͂Ȃ�
	}

	/**
	* �X�g���C�h(���̗v�f�܂ł̋���)���擾����
	*
	* @param accessor   �A�N�Z�T
	* @param bufferView �o�b�t�@�r���[
	*
	* @return �X�g���C�h
	*/
	GLsizei GetByteStride(const json& accessor, const json& bufferView)
	{
		// byteStride����`����Ă�����A���̒l��Ԃ�
		const auto byteStride = bufferView.find("byteStride");
		if (byteStride != bufferView.end()) {
			return byteStride->get<int>();
		}
		// byteStride������`�̏ꍇ�A�v�f1���̃T�C�Y���X�g���C�h�Ƃ���
		// �Q�l: glTF-2.0�d�l 3.6.2.4. Data Alignment
		int componentSize = 1; // �����^�̃T�C�Y
		const int componentType = accessor["componentType"].get<int>();
		switch (componentType) {
		case GL_BYTE:           componentSize = 1; break;
		case GL_UNSIGNED_BYTE:  componentSize = 1; break;
		case GL_SHORT:          componentSize = 2; break;
		case GL_UNSIGNED_SHORT: componentSize = 2; break;
		case GL_UNSIGNED_INT:   componentSize = 4; break;
		case GL_FLOAT:          componentSize = 4; break;
		default:
			LOG_WARNING("glTF�̎d�l�ɂȂ��^%d���g���Ă��܂�", componentType);
			break;
		}
		// �X�g���C�h = �����^�̃T�C�Y * ������
		const int componentCount = GetComponentCount(accessor);
		return componentSize * componentCount;
	}

	/**
	* �f�[�^�̊J�n�ʒu���擾����
	*
	* @param accessor   �A�N�Z�T
	* @param bufferView �o�b�t�@�r���[
	* @param binaryList �o�C�i���f�[�^�z��
	*
	* @return �f�[�^�̊J�n�ʒu
	*/
	GLsizeiptr GetBinaryDataOffset(const json& accessor,
		const json& bufferView, const BinaryList& binaryList)
	{
		const int bufferId = bufferView["buffer"].get<int>();
		const int byteOffset = accessor.value("byteOffset", 0);			// �C���f�b�N�X�f�[�^���̃I�t�Z�b�g
		const int baseByteOffset = bufferView.value("byteOffset", 0);	// �o�b�t�@���̃I�t�Z�b�g
		return binaryList[bufferId].offset + baseByteOffset + byteOffset;
	}

	/**
	* CPU���̃f�[�^�̃A�h���X���擾����
	*
	* @param accessor    �A�N�Z�T
	* @param bufferViews �o�b�t�@�r���[�z��
	* @param binaryList  �o�C�i���f�[�^�z��
	*
	* @return �f�[�^�̃A�h���X
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
	* ���_�A�g���r���[�g��ݒ肷��
	*
	* @param index			���_�A�g���r���[�g�̃C���f�b�N�X
	* @param key			���_�A�g���r���[�g�̖��O
	* @param attributes		���_�A�g���r���[�g
	* @param accessors		�A�N�Z�T
	* @param bufferViews	�o�b�t�@�r���[
	* @param binaryList		�o�C�i���f�[�^�z��
	*
	* @retval true  ���_�A�g���r���[�g��ݒ肵��
	* @retval false ���_�A�g���r���[�g�̃p�����[�^���Ȃ�����
	*/
	bool SetVertexAttribute(
		GLuint index, const char* key,
		const json& attributes, const json& accessors,
		const json& bufferViews, const BinaryList& binaryList)
	{
		glEnableVertexAttribArray(index); // ���_�A�g���r���[�g��L����
		const auto attribute = attributes.find(key);
		if (attribute != attributes.end()) {
			// �A�N�Z�T�ƃo�b�t�@�r���[���擾
			const json& accessor = accessors[attribute->get<int>()];
			const int bufferViewId = accessor["bufferView"].get<int>();
			const json& bufferView = bufferViews[bufferViewId];

			// ���_�A�g���r���[�g�̃p�����[�^���擾
			const GLint componentCount = GetComponentCount(accessor);
			const GLenum componentType = accessor["componentType"].get<int>();
			const GLboolean normalized = accessor.value("normalized", false);
			const GLsizei byteStride = GetByteStride(accessor, bufferView);
			const GLsizeiptr offset = GetBinaryDataOffset(accessor, bufferView, binaryList);

			// VAO�ɒ��_�A�g���r���[�g��ݒ�
			glVertexAttribPointer(index, componentCount,
				componentType, normalized, byteStride, reinterpret_cast<void*>(offset));

			return true;
		}
		return false;
	}

	/**
	* VAO�Ƀf�t�H���g�̒��_�A�g���r���[�g��ݒ肷��
	*
	* @param index  �ݒ��̒��_�A�g���r���[�g�ԍ�
	* @param size   �f�[�^�̗v�f��
	* @param offset �f�[�^�̈ʒu
	* @param vbo    ���_�f�[�^��ێ�����VBO
	*/
	void SetDefaultAttribute(GLuint index, GLint size, GLuint offset, GLuint vbo)
	{
		glVertexAttribFormat(index, size, GL_FLOAT, GL_FALSE, offset);
		glVertexAttribBinding(index, index);

		/* �X�g���C�h�Ƀ[�����w��ł��邱�Ƃł��ׂĂ̒��_�C���f�b�N�X�ɑ΂��ē������_�f�[�^���g���� */
		glBindVertexBuffer(index, vbo, 0, 0);
	}

	/**
	* �e�N�X�`����ǂݍ���
	*
	* @param objectName  �e�N�X�`�����̃L�[������
	* @param gltf        glTF�t�@�C����JSON�I�u�W�F�N�g
	* @param parent      �e�N�X�`����������JSON�I�u�W�F�N�g
	* @param foldername  glTF�t�@�C��������t�H���_��
	*
	* @return �ǂݍ��񂾃e�N�X�`���A�܂���texDefault
	*/
	TexturePtr LoadTexture(
		const char* objectName, const json& gltf, const json& parent, const std::string& foldername)
	{
		// �e�N�X�`�������擾
		const auto textureInfo = parent.find(objectName);
		if (textureInfo == parent.end()) {
			return nullptr;
		}

		// �e�N�X�`���ԍ����擾
		const auto textures = gltf.find("textures");
		const int textureNo = textureInfo->at("index").get<int>();
		if (textures == gltf.end() || textureNo >= textures->size()) {
			return nullptr;
		}

		// �C���[�W�\�[�X�ԍ����擾
		const json& texture = textures->at(textureNo);
		const auto source = texture.find("source");
		if (source == texture.end()) {
			return nullptr;
		}

		// �C���[�W�ԍ����擾
		const auto images = gltf.find("images");
		const int imageNo = source->get<int>();
		if (images == gltf.end() || imageNo >= images->size()) {
			return nullptr;
		}

		// �t�@�C�������擾
		const json& image = images->at(imageNo);
		const auto imageUri = image.find("uri");
		if (imageUri == image.end()) {
			return nullptr;
		}

		// �t�H���_����ǉ����A�g���q��tga�ɕύX
		std::filesystem::path uri = imageUri->get<std::string>();
		std::filesystem::path filename = foldername;
		filename /= uri.parent_path();	// �t�H���_��
		filename /= uri.stem();			// �e�N�X�`����
		filename += ".tga";				// �g���q��tga�ɕύX

		// �e�N�X�`����ǂݍ���
		return TextureManager::GetTexture(filename.string().c_str(), Usage::forGltf);
	}

	/**
	* JSON�̔z��f�[�^��vec3�ɕϊ�����
	*
	* @param json �ϊ����ƂȂ�z��f�[�^
	*
	* @return json��ϊ����Ăł���vec3�̒l
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
	* JSON�̔z��f�[�^��quat�ɕϊ�����
	*
	* @param json �ϊ����ƂȂ�z��f�[�^
	*
	* @return json��ϊ����Ăł���quat�̒l
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
	* JSON�̔z��f�[�^��mat4�ɕϊ�����
	*
	* @param json �ϊ����ƂȂ�z��f�[�^
	*
	* @return json��ϊ����Ăł���mat4�̒l
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
	* �m�[�h�̃��[�J�����W�ϊ��s����v�Z����
	*
	* @param node gltf�m�[�h
	*
	* @return node�̃��[�J�����W�ϊ��s��
	*/
	mat4 GetLocalMatrix(const json& node)
	{
		// �s��f�[�^������ꍇ�A�s��f�[�^��ǂݎ���ĕԂ�
		const auto matrix = node.find("matrix");
		if (matrix != node.end()) {
			return GetMat4(*matrix);
		}

		// �s��f�[�^���Ȃ��ꍇ�A
		// �X�P�[������]�����s�ړ��̏��œK�p�����s���Ԃ�
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
	* ���b�V�������m�[�h�����X�g�A�b�v����
	*/
	void GetMeshNodeList(const GltfNode* node, std::vector<const GltfNode*>& list)
	{
		if (node->mesh >= 0) {
			// ���b�V���������Ă���
			list.push_back(node);
		}
		for (const GltfNode* child : node->children) {
			// �S�Ă̎q�m�[�h�ɑ΂��čs��
			GetMeshNodeList(child, list);
		}
	}

	/**
	* �A�j���[�V�����`���l�����쐬����
	*
	* @param pTimes       �����̔z��̃A�h���X
	* @param pValues      �l�̔z��̃A�h���X
	* @param inputCount   �z��̗v�f��
	* @param targetNodeId �l�̓K�p�ΏۂƂȂ�m�[�hID
	* @param interp       ��ԕ��@
	*
	* @return �쐬�����A�j���[�V�����`���l��
	*/
	template<typename T>
	GltfChannel<T> MakeAnimationChannel(
		const GLfloat* pTimes, const void* pValues, size_t inputCount,
		int targetNodeId, GltfInterpolation interp)
	{
		// �����ƒl�̔z�񂩂�L�[�t���[���z����쐬
		const T* pData = static_cast<const T*>(pValues);
		GltfChannel<T> channel;
		channel.keyframes.resize(inputCount);
		for (int i = 0; i < inputCount; ++i) {
			channel.keyframes[i] = { pTimes[i], pData[i] };
		}

		// �K�p�Ώۃm�[�hID�ƕ�ԕ��@��ݒ�
		channel.targetNodeId = targetNodeId;
		channel.interpolation = interp;

		return channel; // �쐬�����`���l����Ԃ�
	}

} // unnamed namespace


/**
* �R���X�g���N�^
*
* @param bufferCapacity �t�@�C���i�[�p�o�b�t�@�̍ő�o�C�g��
* @param maxMatrixCount �A�j���[�V�����pSSBO�Ɋi�[�ł���ő�s��
*/
GltfFileBuffer::GltfFileBuffer(size_t bufferCapacity, size_t maxMatrixCount)
{
	// GPU���������m�ۂ��A�������ݐ�p�Ƃ��ă}�b�v����
	buffer = BufferObject::Create(bufferCapacity, nullptr,
		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	pBuffer = static_cast<uint8_t*>(glMapNamedBuffer(*buffer, GL_WRITE_ONLY));
	/* glTF��VBO��IBO�ɕ����邱�Ƃ͂����A
	   ���_�f�[�^���C���f�b�N�X�f�[�^�������o�b�t�@�I�u�W�F�N�g�Ɋi�[���� */

	// �o�b�t�@�̐擪�Ƀ_�~�[�f�[�^��ݒ�
	const DefaultVertexData defaultData;
	memcpy(pBuffer, &defaultData, sizeof(defaultData));
	curBufferSize = sizeof(defaultData);
	/* �e�N�X�`�����W�Ȃǂ������Ȃ�glTF�t�@�C����VAO�ɂ����̃f�[�^�����蓖�Ă� */

	// �A�j���[�V�����̎p���s��p�o�b�t�@���쐬
	animationBuffer = MappedBufferObject::Create(maxMatrixCount * sizeof(mat4),
		GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	tmpAnimationBuffer.reserve(maxMatrixCount);
}


/**
* �A�j���[�V�������b�V���̕`��p�f�[�^�����ׂč폜
*
* �t���[���̍ŏ��ɌĂяo�����ƁB
*/
void GltfFileBuffer::ClearAnimationBuffer()
{
	tmpAnimationBuffer.clear();
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^��ǉ�
*
* @param matBones SSBO�ɒǉ�����p���s��̔z��
+*
* @return matBones�p�Ɋ��蓖�Ă�ꂽSSBO�͈̔�
*/
GltfFileRange GltfFileBuffer::AddAnimationMatrices(
	const AnimationMatrices& matBones)
{
	const GLintptr offset =
		static_cast<GLintptr>(tmpAnimationBuffer.size() * sizeof(mat4));
	tmpAnimationBuffer.insert(
		tmpAnimationBuffer.end(), matBones.begin(), matBones.end());

	// SSBO�̃I�t�Z�b�g�A���C�����g�����𖞂������߂ɁA256�o�C�g���E(mat4��4��)�ɔz�u����B
	// 256��OpenGL�d�l�ŋ������ő�l�B
	tmpAnimationBuffer.resize(((tmpAnimationBuffer.size() + 3) / 4) * 4);
	return { offset, matBones.size() * sizeof(mat4) };
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^��GPU�������ɃR�s�[
*/
void GltfFileBuffer::UploadAnimationBuffer()
{
	if (tmpAnimationBuffer.empty()) {
		return; // �]������f�[�^���Ȃ��ꍇ�͉������Ȃ�
	}
	animationBuffer->WaitSync();
	uint8_t* p = animationBuffer->GetMappedAddress();
	memcpy(p, tmpAnimationBuffer.data(), tmpAnimationBuffer.size() * sizeof(mat4));
	animationBuffer->SwapBuffers();
}

/**
* �A�j���[�V�������b�V���̕`��Ɏg��SSBO�̈�����蓖�Ă�
*
* @param bindingPoint �o�C���f�B���O�|�C���g
* @param range        �o�C���h����͈�
*/
void GltfFileBuffer::BindAnimationBuffer(
	GLuint bindingPoint, const GltfFileRange& range)
{
	if (range.size > 0) {
		animationBuffer->Bind(bindingPoint, range.offset, range.size);
	}
}

/**
* �A�j���[�V�������b�V���̕`��Ɏg��SSBO�̈�̊��蓖�Ă���������
*/
void GltfFileBuffer::UnbindAnimationBuffer(GLuint bindingPoint)
{
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingPoint, 0, 0, 0);
}


/**
* glTF�t�@�C������GltfFile�I�u�W�F�N�g���쐬����
*
* @param filename glTF�t�@�C����
*
* @retval nullptr�ȊO  filename����쐬�����t�@�C���I�u�W�F�N�g
* @retval nullptr     �ǂݍ��ݎ��s
*/
GltfFilePtr GltfFileBuffer::LoadGltf(const char* filename)
{
	// glTF�t�@�C����ǂݍ���
	std::vector<char> buf = ReadFile(filename);
	if (buf.empty()) {
		return nullptr;
	}
	buf.push_back('\0'); // �e�L�X�g�I�[��ǉ�

	// �t�H���_�������o���ifilesystem���C�u������parent_path�֐��F�t�H���_�������擾����j
	const std::string foldername =
		std::filesystem::path(filename).parent_path().string() + '/';

	// JSON�����
	auto p = Parse(buf.data(), foldername.c_str());
	if (!p) {
		LOG_ERROR("'%s'�̓ǂݍ��݂Ɏ��s���܂���", filename);
		return p;
	}

	// gltf�t�@�C�����𒊏o����
	// '/' ����؂蕶���Ƃ��ĕ�����𕪊�
	std::string fname = filename;
	size_t lastSlash = fname.find_last_of("/");
	std::string gltffile = fname.substr(lastSlash + 1);	// ��Fbox.gltf

	// �쐬�����t�@�C����A�z�z��ɒǉ�
	p->name = gltffile;
	files.emplace(gltffile, p);

	// �ǂݍ��񂾃t�@�C�����ƃ��b�V�������f�o�b�O���Ƃ��ďo��
	LOG("%s��ǂݍ��݂܂���", filename);
	for (size_t i = 0; i < p->meshes.size(); ++i) {
		LOG(R"(  meshes[%d]="%s")", i, p->meshes[i].name.GetName().c_str());
	}

	// �ǂݍ��񂾃A�j���[�V���������f�o�b�O���Ƃ��ďo��
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
* glTF�t�@�C���̎擾
*
* @param name glTf�t�@�C���̖��O
*
* @return ���O��name�ƈ�v����glTf�t�@�C��
*/
GltfFilePtr GltfFileBuffer::GetGltfFile(const char* name) const
{
	auto itr = files.find(MyFName(name));
	if (itr != files.end()) {
		//�t�@�C�����ɑΉ�����glTf�t�@�C����Ԃ�
		return itr->second;
	}
	return nullptr;
}

/**
* JSON�e�L�X�g����GltfFile�I�u�W�F�N�g���쐬����
*
* @param text       glTF��JSON�e�L�X�g
* @param foldername ���\�[�X�ǂݍ��ݗp�̃t�H���_
*
* @retval nullptr�ȊO  filename����쐬�����t�@�C���I�u�W�F�N�g
* @retval nullptr     �ǂݍ��ݎ��s
*/
GltfFilePtr GltfFileBuffer::Parse(const char* text, const char* foldername)
{
	// JSON���
	json gltf = json::parse(text, nullptr, false);
	if (gltf.is_discarded()) {
		LOG_ERROR("JSON�̉�͂Ɏ��s���܂���");
		return nullptr;
	}

	// �o�C�i���t�@�C����ǂݍ���
	const GLsizeiptr prevBufferSize = curBufferSize;
	const json& buffers = gltf["buffers"];	// �o�C�i���t�@�C�����̔z����擾
	BinaryList binaryList(buffers.size());

	for (size_t i = 0; i < buffers.size(); ++i) {
		const auto uri = buffers[i].find("uri");
		if (uri == buffers[i].end()) {
			continue;
		}

		// �t�@�C����ǂݍ���
		const std::string binPath = foldername + uri->get<std::string>();
		binaryList[i].bin = ReadFile(binPath.c_str());
		if (binaryList[i].bin.empty()) {
			curBufferSize = prevBufferSize; // �ǂݍ��񂾃f�[�^���Ȃ��������Ƃɂ���
			return nullptr; // �o�C�i���t�@�C���̓ǂݍ��݂Ɏ��s
		}

		// �o�C�i���f�[�^��GPU������(�R���X�g���N�^�ō쐬�����o�b�t�@�I�u�W�F�N�g)�ɃR�s�[
		memcpy(pBuffer + curBufferSize, binaryList[i].bin.data(), binaryList[i].bin.size());

		// �I�t�Z�b�g���X�V
		binaryList[i].offset = curBufferSize;
		curBufferSize += binaryList[i].bin.size();
	}


	// �t�@�C���I�u�W�F�N�g���쐬
	GltfFilePtr file = std::make_shared<GltfFile>();

	// ���[�g�m�[�h�̎p������s����擾
	const auto nodes = gltf.find("nodes");
	if (nodes != gltf.end()) {
		// �m�[�h�����݂��邩�`�F�b�N
		if (!nodes->is_array() || nodes->empty()) {
			LOG_ERROR("glTF�t�@�C���̎d�l�ᔽ�F�m�[�h����0");
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


	// ���b�V�����쐬
	const json& accessors = gltf["accessors"];
	const json& bufferViews = gltf["bufferViews"];
	const json& meshes = gltf["meshes"];
	file->meshes.reserve(meshes.size());
	for (const json& jsonMesh : meshes) {
		// ���b�V�������擾
		GltfMesh mesh;
		mesh.name = jsonMesh.value("name", "<default>");	// �L�[�����݂��Ȃ��ꍇ�� <default> ��ݒ肷��

		// �v���~�e�B�u���쐬
		const json& primitives = jsonMesh["primitives"];
		mesh.primitives.reserve(primitives.size());
		for (const json& jsonPrim : primitives) {
			// VAO���쐬
			GltfPrimitive prim;
			prim.vao = std::make_shared<VertexArrayObject>();

			// VAO��OpenGL�R���e�L�X�g�Ɋ��蓖�Ă�
			glBindVertexArray(*prim.vao);

			// VBO��IBO���AOpenGL�R���e�L�X�g��VAO�̗����Ɋ��蓖�Ă�iglTF�p��VBO��IBO�̓o�b�t�@�����L���Ă���j
			glBindBuffer(GL_ARRAY_BUFFER, *buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer);


			// �v���~�e�B�u�̃p�����[�^���擾
			{
				// �v���~�e�B�u�̎��
				prim.mode = jsonPrim.value("mode", GL_TRIANGLES);


				// �C���f�b�N�X���ƌ^
				const int accessorId = jsonPrim["indices"].get<int>();
				const json& accessor = accessors[accessorId];

				prim.count = accessor["count"].get<int>();			// �v���~�e�B�u�̒��_��
				prim.type = accessor["componentType"].get<int>();	// �C���f�b�N�X�f�[�^�̌^


				// �C���f�b�N�X�f�[�^�̊J�n�ʒu
				const int bufferViewId = accessor["bufferView"].get<int>();
				const json& bufferView = bufferViews[bufferViewId];


				// �o�b�t�@���̃C���f�b�N�X�f�[�^�̊J�n�ʒu
				prim.indices = reinterpret_cast<const GLvoid*>(
					GetBinaryDataOffset(accessor, bufferView, binaryList));
			}


			const json& attributes = jsonPrim["attributes"];

			// ���_�A�g���r���[�g(���_���W)���擾
			const bool hasPosition = SetVertexAttribute(AttribIndex::position, "POSITION",
				attributes, accessors, bufferViews, binaryList);
			if (!hasPosition) {
				// �p�����[�^���Ȃ������ꍇ�̓f�t�H���g���_�f�[�^�����蓖�Ă�
				SetDefaultAttribute(AttribIndex::position,
					3, offsetof(DefaultVertexData, position), *buffer);
			}

			// ���_�A�g���r���[�g(�e�N�X�`�����W)���擾
			const bool hasTexcoord0 = SetVertexAttribute(AttribIndex::texcoord0, "TEXCOORD_0",
				attributes, accessors, bufferViews, binaryList);
			if (!hasTexcoord0) {
				// �p�����[�^���Ȃ������ꍇ�̓f�t�H���g���_�f�[�^�����蓖�Ă�
				SetDefaultAttribute(AttribIndex::texcoord0,
					2, offsetof(DefaultVertexData, texcoord0), *buffer);
			}

			// ���_�A�g���r���[�g(�@��)���擾
			const bool hasNormal = SetVertexAttribute(AttribIndex::normal, "NORMAL",
				attributes, accessors, bufferViews, binaryList);
			if (!hasNormal) {
				// �p�����[�^���Ȃ������ꍇ�̓f�t�H���g���_�f�[�^�����蓖�Ă�
				SetDefaultAttribute(AttribIndex::normal,
					3, offsetof(DefaultVertexData, normal), *buffer);
			}

			// ���_�A�g���r���[�g(�^���W�F���g)���擾
			const bool hasTangent = SetVertexAttribute(AttribIndex::tangent, "TANGENT",
				attributes, accessors, bufferViews, binaryList);
			if (!hasTangent) {
				// �p�����[�^���Ȃ������ꍇ�̓f�t�H���g���_�f�[�^�����蓖�Ă�
				SetDefaultAttribute(AttribIndex::tangent,
					4, offsetof(DefaultVertexData, tangent), *buffer);
			}

			// ���_�A�g���r���[�g(�W���C���g�ԍ�)���擾
			const auto hasJoints = SetVertexAttribute(AttribIndex::joints0, "JOINTS_0",
				attributes, accessors, bufferViews, binaryList);
			if (!hasJoints) {
				SetDefaultAttribute(AttribIndex::joints0,
					4, offsetof(DefaultVertexData, joints0), *buffer);
			}

			// ���_�A�g���r���[�g(�W���C���g�E�F�C�g)���擾
			const auto hasWeights = SetVertexAttribute(AttribIndex::weights0, "WEIGHTS_0",
				attributes, accessors, bufferViews, binaryList);
			if (!hasWeights) {
				SetDefaultAttribute(AttribIndex::weights0,
					4, offsetof(DefaultVertexData, weights0), *buffer);
			}

			// �v���~�e�B�u���g�p����}�e���A���ԍ����擾
			prim.materialNo = jsonPrim.value("material", 0);

			// VAO, VBO, IBO��OpenGL�R���e�L�X�g�ւ̊��蓖�Ă�����
			glBindVertexArray(0);	/*�����VAO����������iVBO,IBO���ɉ�ꂷ���VAO�ւ̃o�C���h����������Ă��܂����߁j��*/
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// �쐬�����v���~�e�B�u��z��ɒǉ�
			mesh.primitives.push_back(prim);
		}

		// �쐬�������b�V���I�u�W�F�N�g��z��ɒǉ�
		file->meshes.push_back(mesh);
	}


	// �}�e���A�����쐬
	const auto materials = gltf.find("materials");
	if (materials != gltf.end()) {
		file->materials.reserve(materials->size());
		for (const json& material : *materials) {
			MaterialPtr m = std::make_shared<Material>();

			// ���O��ݒ�
			m->name = material.value("name", std::string());

			// �J���[��ݒ�
			const auto pbr = material.find("pbrMetallicRoughness");	// �J���[���
			if (pbr != material.end()) {
				// �}�e���A���J���[���擾
				m->baseColor = vec4(1);
				const auto baseColorFactor = pbr->find("baseColorFactor");
				if (baseColorFactor != pbr->end()) {
					for (int i = 0; i < baseColorFactor->size(); ++i) {
						m->baseColor[i] = baseColorFactor->at(i).get<float>();
					}
				}

				// �J���[�e�N�X�`����ǂݍ���
				m->texBaseColor = 
					LoadTexture("baseColorTexture", gltf, *pbr, foldername);

				// �e�N�X�`����������Ȃ������ꍇ�AWhite.tga��ݒ肵�Ă���
				if (!m->texBaseColor) {
					m->texBaseColor = TextureManager::GetTexture("White.tga");
				}
			} // if pbr

			// �@���e�N�X�`����ǂݍ���
			m->texNormal = 
				LoadTexture("normalTexture", gltf, material, foldername);

			// �����J���[�̎擾
			m->emission = vec3(0);
			const auto emissiveFactor = material.find("emissiveFactor");
			if (emissiveFactor != material.end()) {
				for (int i = 0; i < emissiveFactor->size(); ++i) {
					m->emission[i] = emissiveFactor->at(i).get<float>();
				}
			}
			// �����e�N�X�`����ǂݍ���
			m->texEmission = 
				LoadTexture("emissiveTexture", gltf, material, foldername);

			// �쐬�����}�e���A����ǉ�
			file->materials.push_back(m);
		}
	} // if materials

	// �m�[�h���쐬
	if (nodes != gltf.end()) {
		file->nodes.resize(nodes->size());
		for (size_t i = 0; i < nodes->size(); ++i) {
			const json& node = nodes->at(i);
			GltfNode& n = file->nodes[i];
			n.name = node.value("name", std::string());
			n.mesh = node.value("mesh", -1);
			n.skin = node.value("skin", -1);

			//LOG(n.name.GetName().c_str());	// TODO:�{�[�����\��

			// �q�m�[�h���擾���A�q�m�[�h�ɑ΂��Đe�m�[�h��ݒ�
			const auto children = node.find("children");
			if (children != node.end()) {
				n.children.resize(children->size());
				for (int b = 0; b < children->size(); ++b) {
					const json& child = children->at(b);
					const int childId = child.get<int>();
					n.children[b] = &file->nodes[childId]; // �q�m�[�h��ǉ�
					n.children[b]->parent = &n; // �e�m�[�h��ݒ�
				}
			}
			// ���[�J�����W�ϊ��s����v�Z
			n.matLocal = GetLocalMatrix(node);
		}

		// �e�����ǂ��ăO���[�o�����W�ϊ��s����v�Z����
		for (GltfNode& e : file->nodes) {
			e.matGlobal = e.matLocal;
			const GltfNode* parent = e.parent;
			while (parent) {
				e.matGlobal = parent->matLocal * e.matGlobal;
				parent = parent->parent;
			}
		}
	} // if nodes

	// �V�[�����쐬
	const auto scenes = gltf.find("scenes");
	if (scenes != gltf.end()) {
		// �e�V�[���ɕ\������m�[�h�z����擾
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

	// �X�L�����擾����
	const auto skins = gltf.find("skins");
	if (skins != gltf.end()) {
		file->skins.resize(skins->size());
		for (size_t skinId = 0; skinId < skins->size(); ++skinId) {
			// �X�L������ݒ�
			const json& skin = skins->at(skinId);
			GltfSkin& s = file->skins[skinId];
			s.name = skin.value("name", std::string());

			// �t�o�C���h�|�[�Y�s��i���f���̃��[�J�����W�n����{�[���̃��[�J�����W�n�ɕϊ�����s��j�̃A�h���X���擾
			const mat4* inverseBindMatrices = nullptr;
			const auto ibm = skin.find("inverseBindMatrices");
			if (ibm != skin.end()) {
				const int ibmId = ibm->get<int>();
				inverseBindMatrices = static_cast<const mat4*>(
					GetBinaryDataAddress(accessors[ibmId], bufferViews, binaryList));
			}

			// �֐߃f�[�^���擾
			const json& joints = skin["joints"];
			s.joints.resize(joints.size());
			for (size_t jointId = 0; jointId < joints.size(); ++jointId) {
				auto& j = s.joints[jointId];
				j.nodeId = joints[jointId].get<int>();
				// �t�o�C���h�|�[�Y�s�񂪖��w��̏ꍇ�͒P�ʍs����g��(glTF�d�l 5.28.1)
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
			// ���O��ݒ�
			const json& animation = animations->at(animeId);
			GltfAnimationPtr a = std::make_shared<GltfAnimation>();
			a->name = animation.value("name", std::string());

			// �`���l���z��̗e�ʂ�\��
			// ��ʓI�ɁA���s�ړ��E��]�E�g��k����3�̓Z�b�g�Ŏw�肷��̂ŁA
			// �e�`���l���z��̃T�C�Y�́u���`���l���� / 3�v�ɂȂ�\���������B
			// ���S�̂��߁A�\���T�C�Y���K��1�ȏ�ɂȂ�悤��1�𑫂��Ă���B
			const json& channels = animation["channels"];
			// 3�̃`���l���z��̗\���T�C�Y�i�\���T�C�Y��0�ɂȂ邱�Ƃ�h������1�𑫂��j
			const size_t predictedSize = channels.size() / 3 + 1;
			a->translations.reserve(predictedSize);
			a->rotations.reserve(predictedSize);
			a->scales.reserve(predictedSize);

			// �S�Ẵm�[�h���u�A�j���[�V�����Ȃ��v�Ƃ��ēo�^
			a->staticNodes.resize(nodes->size());	// �����̍������̂��߂Ɏg�p
			for (int i = 0; i < nodes->size(); ++i) {
				a->staticNodes[i] = i;
			}

			// �`���l���z���ݒ�
			const json& samplers = animation["samplers"];
			a->totalTime_s = 0;
			for (const json& e : channels) {
				// �����̔z����擾
				const json& sampler = samplers[e["sampler"].get<int>()];
				const json& inputAccessor = accessors[sampler["input"].get<int>()];
				const GLfloat* pTimes = static_cast<const GLfloat*>(
					GetBinaryDataAddress(inputAccessor, bufferViews, binaryList));	// TODO:���S�̂��߂�componentType��GL_FLOAT�ł��邱�Ƃ��`�F�b�N����

				// �Đ����Ԃ��v�Z
				a->totalTime_s =
					std::max(a->totalTime_s, inputAccessor["max"][0].get<float>());

				// �l�̔z����擾
				const json& outputAccessor = accessors[sampler["output"].get<int>()];
				const void* pValues =
					GetBinaryDataAddress(outputAccessor, bufferViews, binaryList);

				// ��ԕ��@���擾
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

				// �����ƒl�̔z�񂩂�`���l�����쐬���Apath�ɑΉ�����z��ɒǉ�
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
				a->staticNodes[targetNodeId] = -1; // �A�j���[�V��������
			}

			// �u�A�j���[�V��������v�����X�g����폜����
			const auto itr = std::remove(
				a->staticNodes.begin(), a->staticNodes.end(), -1);
			a->staticNodes.erase(itr, a->staticNodes.end());
			a->staticNodes.shrink_to_fit();

			// �A�j���[�V������ݒ�
			file->animations[animeId] = a;
		}
	} // if animations

	return file; // �쐬�����t�@�C���I�u�W�F�N�g��Ԃ�
}

