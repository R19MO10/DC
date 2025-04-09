/**
* @file Emission.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition; // ���[���h���W
layout(location=1) in vec2 inTexcoord; // �e�N�X�`�����W

// �o�͂���F�f�[�^
out vec4 outColor;

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texColor;	// ��{�F�e�N�X�`��
layout(binding=1) uniform sampler2D texEmission;// �����F�e�N�X�`��

// �v���O��������̓���
layout(location=99) uniform float alphaCutoff;	// �t���O�����g��j�����鋫�E�l
layout(location=100) uniform vec4 color;		// ���̂̐F
layout(location=101) uniform int useTexEmission;// �G�~�b�V�����e�N�X�`���̗L��
layout(location=102) uniform vec4 emission;		// ���̂̔����F


const float Gamma = 2.2;	// �K���}�l(2.2��CRT���j�^�[�̃K���}�l)


/**
* �G���g���[�|�C���g
*/
void main()
{
	// �J���[�e�N�X�`�����瓧���x���擾����
	float alpha = texture(texColor, inTexcoord).a;

	// �J�b�g�I�t�l���w�肳��Ă���ꍇ�A�A���t�@�����̒l�����̏ꍇ�͕`����L�����Z��
	if (alphaCutoff > 0) {
		// �J���[�e�N�X�`���œ����ȏ��̓t���O�����g��j������
		if (alpha < alphaCutoff) {
			discard; // �t���O�����g��j��(�L�����Z��)
		}
		alpha = 1; // �j������Ȃ�������s�����ɂ���
	}

	outColor.a = alpha * color.a;
	
	// �����F�𔽉f
	if (useTexEmission == 1) {	// �G�~�b�V�����e�N�X�`���̗L��
		outColor.rgb = texture(texEmission, inTexcoord).rgb * emission.rgb * emission.a;
	} else {
		outColor.rgb = emission.rgb * emission.a;
	}

	// �K���}�␳���s��
	outColor.rgb = pow(outColor.rgb, vec3(1 / Gamma));
}