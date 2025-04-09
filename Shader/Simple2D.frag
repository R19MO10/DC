/**
* @file Simple2D.frag
*/
#version 450

// �V�F�[�_�ւ̓���
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
  outColor.a = 1;
}