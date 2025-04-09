/**
* @file ParticleEmission.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec4 inColor;		// ��{�F
layout(location=1) in vec2 inTexcoord;	// �e�N�X�`�����W

// �o�͂���F�f�[�^
out vec4 outColor;

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texColor;	// ��{�F�e�N�X�`��


/**
* �G���g���[�|�C���g
*/
void main()
{
	outColor = texture(texColor, inTexcoord);

	// �����F�𔽉f
	if (inColor.a > 1) {	// �G�~�b�V�������x�̗L��
		outColor.rgb *= inColor.rgb;
		outColor.rgb *= (inColor.a - 1) * 0.1f;
	}
	else {
		outColor.rgb = vec3(0);
	}
}