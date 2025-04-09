/**
* @file Animation.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition;  // ���_���W
layout(location=1) in vec2 inTexcoord;  // �e�N�X�`�����W
layout(location=4) in vec4 inJoints;	// �g�p����p���s��̔ԍ�
layout(location=5) in vec4 inWeights;	// �p���s��̏d�v�x

// �V�F�[�_����̏o��
layout(location=0) out vec3 outPosition;    // ���[���h���W
layout(location=1) out vec2 outTexcoord;    // �e�N�X�`�����W

// �v���O��������̓���
layout(location=1) uniform mat4 cameraTransformMatrix;  // �J�����̍��W�ϊ��s��

// x: ����p�ɂ�鐅���g�嗦
// y: ����p�ɂ�鐂���g�嗦
// z: ���ߖ@�p�����[�^A
// w: ���ߖ@�p�����[�^B
layout(location=2) uniform vec4 cameraInfo;

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
	outTexcoord = inTexcoord;

	// �p���s��ɃE�F�C�g���|���ĉ��Z�������A���f���s����쐬
	mat4 matModel =
		matBones[int(inJoints.x)] * inWeights.x +
		matBones[int(inJoints.y)] * inWeights.y +
		matBones[int(inJoints.z)] * inWeights.z +
		matBones[int(inJoints.w)] * inWeights.w;

	// �E�F�C�g�����K������Ă��Ȃ��ꍇ�̑΍�([3][3]��1.0�ɂȂ�Ƃ͌���Ȃ�)
	matModel[3][3] = dot(inWeights, vec4(1));

	// �g��k���Ɖ�]�ƕ��s�ړ�
	gl_Position = matModel * vec4(inPosition, 1);

	outPosition = gl_Position.xyz;

	// ���[���h���W�n����r���[���W�n�ɕϊ�
	vec3 pos = outPosition - vec3(cameraTransformMatrix[3]);
	gl_Position.xyz = pos * mat3(cameraTransformMatrix);
	
	// ����p�𔽉f
	gl_Position.xy *= cameraInfo.xy;
	
	// ���ߖ@��L���ɂ���
	gl_Position.w = -gl_Position.z;
  
	// �[�x�l��␳�i�[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ɂ���j
	gl_Position.z = -gl_Position.z * cameraInfo.w + cameraInfo.z; 
}