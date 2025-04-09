/**
* @file ShadowAnimation.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition;  // ���_���W
layout(location=1) in vec2 inTexcoord;  // �e�N�X�`�����W
layout(location=4) in vec4 inJoints;	// �g�p����p���s��̔ԍ�
layout(location=5) in vec4 inWeights;	// �p���s��̏d�v�x

// �V�F�[�_����̏o��
layout(location=1) out vec2 outTexcoord; // �e�N�X�`�����W

// �v���O��������̓���
layout(location=3) uniform mat4 viewProjectionMatrix; // �r���[�v���W�F�N�V�����s��

// �X�P���^���A�j���[�V�����pSSBO
layout(std430, binding=1) buffer AnimationDataBlock
{
	mat4 matBones[]; // �p���s��̔z��
};


/**
* �G���g���[�|�C���g
*/
void main()
{
	// �e�N�X�`�����W��ݒ�
	outTexcoord = inTexcoord;
	
	// �p���s��ɃE�F�C�g���|���ĉ��Z�������A���f���s����쐬
	mat4 matModel =
		matBones[int(inJoints.x)] * inWeights.x +
		matBones[int(inJoints.y)] * inWeights.y +
		matBones[int(inJoints.z)] * inWeights.z +
		matBones[int(inJoints.w)] * inWeights.w;

	// �E�F�C�g�����K������Ă��Ȃ��ꍇ�̑΍�([3][3]��1.0�ɂȂ�Ƃ͌���Ȃ�)
	matModel[3][3] = dot(inWeights, vec4(1));

	// ���[�J�����W�n���烏�[���h���W�n�ɕϊ�
	gl_Position = matModel * vec4(inPosition, 1);
	gl_Position = viewProjectionMatrix * gl_Position;
}