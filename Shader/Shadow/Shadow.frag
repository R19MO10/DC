/**
* @file Shadow.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=1) in vec2 inTexcoord; // �e�N�X�`�����W

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texColor;

// �v���O��������̓���
layout(location=99) uniform float alphaCutoff; // �t���O�����g��j�����鋫�E�l


/**
* �G���g���[�|�C���g
*/
void main()
{
	float alpha = texture(texColor, inTexcoord).a;
	
	// �J�b�g�I�t�l���w�肳��Ă���ꍇ�A�A���t�@�����̒l�����̏ꍇ�͕`����L�����Z��
	if (alphaCutoff > 0) {
		if (alpha < alphaCutoff) {
			discard; // �t���O�����g��j��
		}
	}
}