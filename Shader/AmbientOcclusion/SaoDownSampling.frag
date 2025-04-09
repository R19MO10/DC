/**
* @file SaoDownSampling.frag
*/
#version 450

// �o�͂���F�f�[�^
out float outColor;

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texLinearDepth;	// ���`�[�x�l

// �v���O��������̓���
layout(location=200) uniform int mipLevel; // �R�s�[���̃~�b�v���x��


/**
* ���`�������k��
*/
void main()
{
	// 2x2�e�N�Z���̂����A�ł����_�ɋ߂��l��I��
	ivec2 uv = ivec2(gl_FragCoord.xy) * 2;

	// ����4�s�N�Z���̂����A�ł����_�ɋ߂��l��I��
	float d1 = texelFetch(texLinearDepth, uv + ivec2(0, 0), mipLevel).x;
	float d2 = texelFetch(texLinearDepth, uv + ivec2(0, 1), mipLevel).x;
	float d3 = texelFetch(texLinearDepth, uv + ivec2(1, 1), mipLevel).x;
	float d4 = texelFetch(texLinearDepth, uv + ivec2(1, 0), mipLevel).x;
	outColor = min(min(d1, d2), min(d3, d4));
}