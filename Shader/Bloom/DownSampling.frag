/**
* @file DownSampling.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=1) in vec2 inTexcoord;	// �e�N�X�`�����W

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texColor;

// �o�͂���F�f�[�^
out vec4 outColor;


/**
* �k���V�F�[�_
*/
void main()
{
  // �T���v�����O�|�C���g�Əd�݂̔z��
  const float offsets[] = { -2.75, -0.92, 0.92, 2.75 };
  const float weights[] = { 0.1087, 0.3913, 0.3913, 0.1087 };

  // �e�N�Z���T�C�Y���v�Z
  vec2 oneTexel = 1 / vec2(textureSize(texColor, 0));

  // 8x8�s�N�Z���̂ڂ����������s��  
    //�����S����킸���ɓ����ɂ��炵�Ă���
  outColor = vec4(0);
  for (int y = 0; y < 4; ++y) {
    float ty = inTexcoord.y + offsets[y] * oneTexel.y;
    for (int x = 0; x < 4; ++x) {
      float tx = inTexcoord.x + offsets[x] * oneTexel.x;
      outColor += texture(texColor, vec2(tx, ty)) * weights[x] * weights[y];
    }
  }
}