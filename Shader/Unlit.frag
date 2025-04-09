/**
* @file Unlit.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition;	// ���[���h���W
layout(location=1) in vec2 inTexcoord;	// �e�N�X�`�����W

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texColor;	// ��{�F�e�N�X�`��
layout(binding=1) uniform sampler2D texEmission;// �����F�e�N�X�`��

// �o�͂���F�f�[�^
out vec4 outColor;

// �v���O��������̓���
layout(location=99)  uniform float alphaCutoff; // �t���O�����g��j�����鋫�E�l
layout(location=100) uniform vec4 color;		// ���̂̐F
layout(location=101) uniform int useTexEmission;// �G�~�b�V�����e�N�X�`���̗L��
layout(location=102) uniform vec4 emission;		// ���̂̔����F


const float Gamma = 2.2;		// �K���}�l(2.2��CRT���j�^�[�̃K���}�l)


/**
* �G���g���[�|�C���g
*/
void main()
{
	vec4 c = texture(texColor, inTexcoord);

	// �J�b�g�I�t�l���w�肳��Ă���ꍇ�A�A���t�@�����̒l�����̏ꍇ�͕`����L�����Z��
	if (alphaCutoff > 0) {
		if (c.a < alphaCutoff) {
			discard; // �t���O�����g��j��(�L�����Z��)
		}
		c.a = 1; // �j������Ȃ�������s�����ɂ���
    }

	// �e�N�X�`���̃K���}�␳������
	c.rgb = pow(c.rgb, vec3(Gamma));

	outColor = c * color;
	
	// �����F�𔽉f
	outColor.rgb += 
		(useTexEmission == 1 ? // �G�~�b�V�����e�N�X�`���L��
			texture(texEmission, inTexcoord).rgb * emission.rgb : emission.rgb);

	// �K���}�␳���s��
	outColor.rgb = pow(outColor.rgb, vec3(1 / Gamma));

	//�J������Z���W�𐳋K�����āA[-1, 1]�͈̔͂ɕϊ�
	float zNormalized = (gl_FragCoord.z * 2.0) - 1.0;
	//���K�����ꂽZ���W�����̉��߃N���b�v�ʂ͈̔͂ɖ߂�
	gl_FragDepth = (gl_DepthRange.diff * zNormalized + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}