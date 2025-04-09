/**
* @file SaoBlur.frag
*/
#version 450

// �o�͂���F�f�[�^
out vec4 outColor;

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texShield;      // �Օ���
layout(binding=1) uniform sampler2D texLinearDepth; // ���`�[�x�l


// �ڂ����W���̔z��
const float Gaussian[5] = 
  { 0.153170, 0.444893, 0.422649, 0.392902, 0.362970 };

const int BlurRadius = 4;        // �ڂ������a
const int BlurScale = 1;         // �ڂ������a�̊g�嗦
const float EdgeDistance = 0.15; // �������̂Ƃ݂Ȃ��[�x��(�P��=m)(�[�x����15cm�́ANvidia��SAO�̘_���Ŏg�p���ꂽ�l)
const float EdgeSharpness = 1 / EdgeDistance; // ���̗̂֊s���ڂ����Ȃ����߂̌W��
const float Far = 1000;          // �t�@�[���ʂ̐[�x�l


/**
* SAO���v�Z���ʂ͗��󊴂��ڗ����Ă��邽�߁A�摜���ڂ����ė��q���ڗ����Ȃ��悤�ɂ���
*/
void main()
{
	// �s�N�Z���̎Օ����Ɛ[�x�l���擾
	ivec2 uv = ivec2(gl_FragCoord.xy);
	outColor = texelFetch(texShield, uv, 0);
	float depth = texelFetch(texLinearDepth, uv, 0).x;
	
	// �[�x�l���t�@�[���ʈȏ�̏ꍇ�A���̃s�N�Z���͔w�i�Ȃ̂łڂ����Ȃ�
	if (depth >= Far) {
	  return;
	}

	// �o�C���e�����E�t�B���^�����s
    /* �[�x�l�𗘗p�������̗̂֊s������s���A
        �֊s(�Ǝv����)�������c���A�֊s�ł͂Ȃ��������ڂ��� */
    outColor *= Gaussian[0] * Gaussian[0];
    float totalWeight = Gaussian[0] * Gaussian[0];

    /* �T���v�����O�͈͂́u(BlurRadius * 2 + 1)x(BlurRadius * 2 + 1)�s�N�Z���v */
    for (int y = -BlurRadius; y <= BlurRadius; ++y) {
        for (int x = -BlurRadius; x <= BlurRadius; ++x) {
            // ���S�͎擾�ς݂Ȃ̂Ŗ�������
            if (y == 0 && x == 0) {
                continue;
            }
        
            // �T���v���_�̃E�F�C�g(�d�v�x)���擾
            float weight = Gaussian[abs(x)] * Gaussian[abs(y)];
        
            // ���S�Ƃ̐[�x�l�̍��ɂ���ĕ��̂̋��E�𔻒肵�A�E�F�C�g�𒲐�
            /* EdgeDistance�ɋ߂Â��قǏd�v�x���ቺ���A
                EdgeDistance�ȏ�ɂȂ�Əd�v�x��0�ɂȂ� */
            /* �[�x�l�̍���EdgeDistance�ȏ゠��ꍇ�́A
                �T���v���_�͒����s�N�Z���Ƃ͈قȂ镽�ʂɑ����� */
            ivec2 sampleUV = uv + ivec2(x, y) * BlurScale;
            float sampleDepth = texelFetch(texLinearDepth, sampleUV, 0).x;
            weight *= max(0.0, 1.0 - EdgeSharpness * abs(sampleDepth - depth));
        
            // �Օ����ƍ��v�E�F�C�g���X�V
            outColor += texelFetch(texShield, sampleUV, 0) * weight;
            totalWeight += weight;
        }
    }
    
    outColor /= totalWeight;
}