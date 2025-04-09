/**
* @file Shadow.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition; // ���_���W
layout(location=1) in vec2 inTexcoord; // �e�N�X�`�����W

// �V�F�[�_����̏o��
layout(location=1) out vec2 outTexcoord; // �e�N�X�`�����W

// �v���O��������̓���
layout(location=0) uniform mat4 transformMatrix; // �`�惂�f���̍��W�ϊ��s��
layout(location=3) uniform mat4 viewProjectionMatrix; // �r���[�v���W�F�N�V�����s��


/**
* �G���g���[�|�C���g
*/
void main()
{
	// �e�N�X�`�����W��ݒ�
	outTexcoord = inTexcoord;
	
	// ���[�J�����W�n���烏�[���h���W�n�ɕϊ�
	gl_Position = transformMatrix * vec4(inPosition, 1);
	gl_Position = viewProjectionMatrix * gl_Position;
}