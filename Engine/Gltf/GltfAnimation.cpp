/**
* @file GltfAnimation.cpp
*/
#include "GltfAnimation.h"

#include "GltfFile.h"
#include <algorithm>

namespace {

	/**
	* �A�j���[�V�����v�Z�p�̒��ԃf�[�^�^
	*/
	struct NodeMatrix
	{
		MyFName name;	// ���O
		mat4 m = mat4(1);	// �p���s��
		bool isCalculated = false;  // �v�Z�ς݃t���O
	};
	using NodeMatrices = std::vector<NodeMatrix>;
	
	/**
	* �m�[�h�̃O���[�o���p���s����v�Z����
	*/
	const mat4& CalcGlobalNodeMatrix(const std::vector<GltfNode>& nodes,
		const GltfNode& node, NodeMatrices& matrices)
	{
		const intptr_t currentNodeId = &node - &nodes[0];
		NodeMatrix& nodeMatrix = matrices[currentNodeId];

		// �u�v�Z�ς݁v�̏ꍇ�͎����̎p���s���Ԃ�
		if (nodeMatrix.isCalculated) {
			return nodeMatrix.m;
		}

		// �u�v�Z�ς݂łȂ��v�ꍇ�A�e�̎p���s�����������
		if (node.parent) {
			// �e�̍s����擾(�ċA�Ăяo��)
			const mat4& matParent =
				CalcGlobalNodeMatrix(nodes, *node.parent, matrices);

			// �e�̎p���s�������
			nodeMatrix.m = matParent * nodeMatrix.m;
		}

		// �u�v�Z�ς݁v�ɂ���
		nodeMatrix.isCalculated = true;

		// �����̎p���s���Ԃ�
		return nodeMatrix.m;
	}

	/**
	* �`���l����̎w�肵�������̒l�����߂�
	*
	* @param channel �Ώۂ̃`���l��
	* @param time    �l�����߂鎞��
	*
	* @return �����ɑΉ�����l
	*/
	template<typename T>
	T Interpolate(const GltfChannel<T>& channel, float time)
	{
		// time�ȏ�̎��������A�ŏ��̃L�[�t���[��������
		const auto& curOrOver = std::lower_bound(
			channel.keyframes.begin(), channel.keyframes.end(), time,
			[](const GltfKeyframe<T>& keyframe, float time) {
				return keyframe.time < time; });

		// time���擪�L�[�t���[���̎����Ɠ������ꍇ�A�擪�L�[�t���[���̒l��Ԃ�
		if (curOrOver == channel.keyframes.begin()) {
			return channel.keyframes.front().value;
		}

		// time�������L�[�t���[���̎������傫���ꍇ�A�����L�[�t���[���̒l��Ԃ�
		if (curOrOver == channel.keyframes.end()) {
			return channel.keyframes.back().value;
		}

		// time���擪�Ɩ����̊Ԃ������ꍇ
		// �L�[�t���[���Ԃ̎��Ԃɂ�����time�̔䗦���v�Z���A�䗦�ɂ���ĕ�Ԃ����l��Ԃ�
		const auto& prev = curOrOver - 1; // �ЂƂO��(time�����̎���������)�L�[�t���[��
		const float frameTime = curOrOver->time - prev->time;
		const float t = std::clamp((time - prev->time) / frameTime, 0.0f, 1.0f);

		// ����: ���͏��(����)���`��Ԃ����Ă��邪�A�{���͕�ԕ��@�ɂ���ď����𕪂���ׂ�
		if constexpr (std::is_same_v<T, Quaternion>) {
			return slerp(prev->value, curOrOver->value, t);
		}
		else {
			return prev->value * (1 - t) + curOrOver->value * t;
		}
	}
}	// unnamed namespace


/**
* �A�j���[�V������K�p�����p���s����v�Z����
*
* @param file             meshNode�����L����t�@�C���I�u�W�F�N�g
* @param meshNode         ���b�V�������m�[�h
* @param animation        �v�Z�̌��ɂȂ�A�j���[�V����
* @param time             �A�j���[�V�����̍Đ��ʒu
*
* @return �A�j���[�V������K�p�����p���s��̔z��
*/
GltfAnimationMatrices CalcAnimationMatrices(const GltfFilePtr& file,
	const GltfNode* meshNode, const GltfAnimation* animation, float time)
{
	GltfAnimationMatrices matBones;
	if (!file || !meshNode) {
		return matBones;
	}

	// �A�j���[�V�������ݒ肳��Ă��Ȃ��ꍇ...
	if (!animation) {
		// �m�[�h�̃O���[�o�����W�ϊ��s����g��
		size_t size = 1;
		if (meshNode->skin >= 0) {
			size = file->skins[meshNode->skin].joints.size();
		}
		matBones.resize(size, { meshNode->name, meshNode->matGlobal });

		return matBones;
	}

	// �A�j���[�V�������ݒ肳��Ă���ꍇ...
	NodeMatrices matrices;
	const auto& nodes = file->nodes;
	matrices.resize(nodes.size());


	// �A�j���[�V�������Ȃ��m�[�h�̃��[�J���p���s���ݒ�
	for (const auto e : animation->staticNodes) {
		matrices[e].m = nodes[e].matLocal;

		// ���g�̐e�̖��O���R�s�[
		matrices[e].name = nodes[e].name;
	}

	// �A�j���[�V��������m�[�h�̃��[�J���p���s����v�Z
	// (�g��k������]�����s�ړ��̏��œK�p)
	for (const auto& e : animation->translations) {
		const vec3 translation = Interpolate(e, time);
		matrices[e.targetNodeId].m = Mat::Translate(translation);

		// ���O���R�s�[
		matrices[e.targetNodeId].name = nodes[e.targetNodeId].name;
	}
	for (const auto& e : animation->rotations) {
		const Quaternion rotation = Interpolate(e, time);
		matrices[e.targetNodeId].m *= mat4(rotation);
	}
	for (const auto& e : animation->scales) {
		const vec3 scale = Interpolate(e, time);
		matrices[e.targetNodeId].m *= Mat::Scale(scale);
	}

	// �A�j���[�V������K�p�����O���[�o���p���s����v�Z�i���[�J�����W�ϊ��s�񁨃o�C���h�|�[�Y�s��j
	if (meshNode->skin >= 0) {
		for (const auto& joint : file->skins[meshNode->skin].joints) {
			CalcGlobalNodeMatrix(nodes, nodes[joint.nodeId], matrices);
		}
	}
	else {
		// �W���C���g���Ȃ��̂Ń��b�V���m�[�h�����v�Z
		CalcGlobalNodeMatrix(nodes, *meshNode, matrices);
	}

	// �t�o�C���h�|�[�Y�s�������
	if (meshNode->skin >= 0) {
		// glTF��joints�L�[�ɂ̓m�[�h�ԍ����i�[����Ă���
		// �������A���_�f�[�^��JOINTS_n�����ɂ́ujoints�z��̃C���f�b�N�X�v���i�[����Ă���
		// ���ۂɎp���s����g���̂͒��_�f�[�^�Ȃ̂ŁA�p���s���joints�z��̏��ԂŊi�[����
		const auto& joints = file->skins[meshNode->skin].joints;
		matBones.resize(joints.size());
		for (size_t i = 0; i < joints.size(); ++i) {
			const auto& joint = joints[i];

			matBones[i].name = matrices[joint.nodeId].name;
			matBones[i].m = matrices[joint.nodeId].m * joint.matInverseBindPose;
		}
	}
	else {
		// �W���C���g���Ȃ��̂ŋt�o�C���h�|�[�Y�s������݂��Ȃ�
		const size_t nodeId = meshNode - &nodes[0];
		matBones.resize(1, { matrices[nodeId].name, matrices[nodeId].m });
	}

	return matBones;
}
