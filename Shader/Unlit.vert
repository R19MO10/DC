/**
* @file Unlit.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition;  // ���_���W
layout(location=1) in vec2 inTexcoord;  // �e�N�X�`�����W

// �V�F�[�_����̏o��
layout(location=0) out vec3 outPosition;    // ���[���h���W
layout(location=1) out vec2 outTexcoord;    // �e�N�X�`�����W

// �v���O��������̓���
layout(location=0) uniform mat4 transformMatrix;   // �`�惂�f���̍��W�ϊ��s��
layout(location=1) uniform mat4 cameraTransformMatrix;  // �J�����̍��W�ϊ��s��

// x: ����p�ɂ�鐅���g�嗦
// y: ����p�ɂ�鐂���g�嗦
// z: ���ߖ@�p�����[�^A
// w: ���ߖ@�p�����[�^B
layout(location=2) uniform vec4 cameraInfo;


/**
* �G���g���[�|�C���g
*/
void main()
{
	outTexcoord = inTexcoord;

	// �g��k���Ɖ�]�ƕ��s�ړ�
	gl_Position = transformMatrix * vec4(inPosition, 1);

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