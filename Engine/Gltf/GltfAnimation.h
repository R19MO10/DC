/**
* @ file GltfAnination.h
*/
#ifndef GLTFANIMATION_H_INCLUDED
#define GLTFANIMATION_H_INCLUDED
#include "../../Utility/MyFName.h"
#include "../../Math/MyMath.h"
#include <vector>
#include <string>
#include <memory>

// ��s�錾
struct GltfFile;
using  GltfFilePtr = std::shared_ptr<GltfFile>;

/**
* �X�L��
*/
struct GltfSkin
{
	MyFName name; // �X�L����

	// �W���C���g(�{�[��)�f�[�^
	struct Joint {
		int nodeId;
		mat4 matInverseBindPose;
	};
	std::vector<Joint> joints; // �W���C���g�z��
};

/**
* �m�[�h
*/
struct GltfNode
{
	MyFName name;                // �m�[�h��
	int mesh = -1;                   // ���b�V���ԍ�
	int skin = -1;                   // �X�L���ԍ�
	GltfNode* parent = nullptr;      // �e�m�[�h
	std::vector<GltfNode*> children; // �q�m�[�h�z��
	mat4 matLocal = mat4(1);  // ���[�J���s��
	mat4 matGlobal = mat4(1); // �O���[�o���s��
};

/**
* �V�[��
*/
struct GltfScene
{
	std::vector<const GltfNode*> nodes;     // �m�[�h�z��
	std::vector<const GltfNode*> meshNodes; // ���b�V�������m�[�h�݂̂̔z��
};

/**
* �A�j���[�V�����̕�ԕ��@
*/
enum class GltfInterpolation
{
	step,        // ��ԂȂ�
	linear,      // ���`���
	cubicSpline, // 3���X�v���C�����
};

/**
* �A�j���[�V�����̃L�[�t���[��
*/
template<typename T> struct GltfKeyframe
{
	float time; // ����
	T value;    // �K�p����l
};

/**
* �A�j���[�V�����̃`���l��
*/
template<typename T> struct GltfChannel
{
	int targetNodeId;                       // �l��K�p����m�[�hID
	GltfInterpolation interpolation;        // ��ԕ��@
	std::vector<GltfKeyframe<T>> keyframes; // �L�[�t���[���z��
};

/**
* �A�j���[�V����
*/
struct GltfAnimation
{
	MyFName name; // �A�j���[�V������
	std::vector<GltfChannel<vec3>> translations; // ���s�ړ��`���l���̔z��
	std::vector<GltfChannel<Quaternion>> rotations;    // ��]�`���l���̔z��
	std::vector<GltfChannel<vec3>> scales;       // �g��k���`���l���̔z��
	std::vector<int> staticNodes; // �A�j���[�V�������Ȃ��m�[�hID�̔z��
	float totalTime_s = 0;
};
using GltfAnimationPtr = std::shared_ptr<GltfAnimation>;

// �A�j���[�V�����p���W�ϊ��s��̔z��
struct GltfAnimationMat
{
	MyFName name;	// �{�[�����O
	mat4 m;
};
using GltfAnimationMatrices = std::vector<GltfAnimationMat>;

GltfAnimationMatrices CalcAnimationMatrices(const GltfFilePtr& file,
	const GltfNode* meshNode, const GltfAnimation* animation, float time);

#endif // GLTFANIMATION_H_INCLUDED