/**
* @file SaoRestoreDepth.frag
*/
#version 450

// �o�͂���F�f�[�^
out float outColor;

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texDepth;	// �[�x�l�e�N�X�`��

// �v���O��������̓���
layout(location=200) uniform vec2 cameraNearFar;
#define CameraNear	(cameraNearFar.x) // near�v���[���܂ł̋���
#define CameraFar	(cameraNearFar.y) // far�v���[���܂ł̋���


/**
* �[�x�o�b�t�@�̒l����r���[��Ԃ�Z�l�𕜌��i���`�����ւ̕ϊ��j
*/
void main()
{
	// 2x2�e�N�Z���̂����A�ł����_�ɋ߂��l��I��
	/* �T�C�Y���c��1/2(�ʐςł�1/4)�ɏk�����Čv�Z���Ă���̂�
		�e�N�X�`���̃T�C�Y�����炵�A�Ȍ�̏����Ńe�N�X�`���̓ǂݎ��ɂ����鎞�Ԃ�Z�k���邽�� */
	ivec2 uv = ivec2(gl_FragCoord.xy) * 2;
	// ����4�s�N�Z���̂����A�ł����_�ɋ߂��l��I��
	/* �[�x�l�͔���`�̂��ߒP���Ƀs�N�Z���̕��ς��g���Ȃ����� */
	float d1 = texelFetch(texDepth, uv + ivec2(0, 0), 0).x;
	float d2 = texelFetch(texDepth, uv + ivec2(0, 1), 0).x;
	float d3 = texelFetch(texDepth, uv + ivec2(1, 1), 0).x;
	float d4 = texelFetch(texDepth, uv + ivec2(1, 0), 0).x;
	float depth = min(min(d1, d2), min(d3, d4));
	
	// �[�x�l����`�ɖ߂�
	/* �[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ȃp�����[�^A, B */
	const float A = -2 * CameraFar * CameraNear / (CameraFar - CameraNear);
	const float B = (CameraFar + CameraNear) / (CameraFar - CameraNear);
	depth = 2 * depth - 1;
	outColor = A / (depth - B);
}