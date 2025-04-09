/**
* @file HighPassFilter.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=1) in vec2 inTexcoord;	// �e�N�X�`�����W

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texColor;

// �o�͂���F�f�[�^
out vec4 outColor;

// �v���O��������̓���
layout(location=150) uniform vec2 bloomInfo;
#define Bloom_Threshold     (bloomInfo.x) // ���P�x�Ƃ݂Ȃ����邳(�������l)
#define Bloom_Strength      (bloomInfo.y) // �u���[���̋���


/**
* ���P�x�s�N�Z���̐F�������擾����
*/
vec3 GetBrightnessPart(vec3 color)
{
  // RGB�̂����A�����Ƃ����邢�v�f�̒l���s�N�Z���̋P�x�Ƃ���
  float brightness = max(color.r, max(color.g, color.b));

  // ���P�x�����̔䗦���v�Z
  /* �E�ӂ�max��0���Z�G���[��� */
  float ratio = max(brightness - Bloom_Threshold, 0) / max(brightness, 0.00001);

  // ���P�x�������v�Z
  return color * ratio;
}


/**
* ���P�x�����𒊏o���A�k������
*/
void main()
{
    // �e�N�Z���T�C�Y���v�Z
    vec2 oneTexel = 1 / vec2(textureSize(texColor, 0));

    // ���邢�������v�Z�A4x4�s�N�Z���̏k���ڂ�������
    /* ���S�̂��߁A4�|���Ė��邭���� */
    outColor.rgb = GetBrightnessPart(texture(texColor, inTexcoord).rgb) * 4;

    // �l��
    outColor.rgb += GetBrightnessPart(
        texture(texColor, inTexcoord + vec2(-oneTexel.x, oneTexel.y)).rgb); /* ���� */
    outColor.rgb += GetBrightnessPart(
        texture(texColor, inTexcoord + vec2( oneTexel.x, oneTexel.y)).rgb); /* ���� */
    outColor.rgb += GetBrightnessPart(
        texture(texColor, inTexcoord + vec2(-oneTexel.x,-oneTexel.y)).rgb); /* �E�� */
    outColor.rgb += GetBrightnessPart(
        texture(texColor, inTexcoord + vec2( oneTexel.x,-oneTexel.y)).rgb); /* �E�� */

    // �u���[���̋�������Z
    /* �l�𕽋ω��̂��߂� 1/8 �ɂ��Ă��� */
    outColor.rgb *= (1.0 / 8.0) * Bloom_Strength;   

    outColor.a = 1;   /* �g�p���Ȃ�����1 */
}