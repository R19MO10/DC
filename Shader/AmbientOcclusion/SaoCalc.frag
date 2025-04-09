/**
* @file SaoCalc.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=1) in vec2 inTexcoord;  // �e�N�X�`�����W

// �o�͂���F�f�[�^
out float outColor;

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texLinearDepth; // �[�x�e�N�X�`��

// �v���O��������̓���
layout(location=200) uniform vec4 saoInfo;
#define SAO_Radius2             (saoInfo.x) // �T���v�����O���a(m)��2��
#define SAO_Radius_in_Textures  (saoInfo.y) // �T���v�����O���a(�e�N�X�`�����W)
#define SAO_Bias                (saoInfo.z) // ���ʂƂ݂Ȃ��R�T�C���l
#define SAO_Intensity           (saoInfo.w) // AO�̋��x

// NDC���W���r���[���W�ɕϊ����邽�߂̃p�����[�^
layout(location=201) uniform vec2 ndcToView;


const float TwoPI = 3.14159265 * 2; // 2��(360�x)

// SAO�̘_���ɂ��ƁA�T���v���������̑f�����w�肷��ƁASAO�̕i�������シ��
const float SamplingCount = 11;     // �T���v�����i1�s�N�Z����AO�v�Z�Ŏ擾����T���v�����j
const float SpiralTurns = 7;        // �T���v���_�̉�]�񐔁i�T���v�����O�ʒu�����߂闆���̊����񐔁j

const int MaxMipLevel = 3;          // texDepth�Ɋ��蓖�Ă�e�N�X�`���̍ő�~�b�v���x��

// �~�b�v���x�����؂�ւ��n�߂�T���v���_�̋���(2^logMipOffset�s�N�Z��)
const int LogMipOffset = 3;


/**
* �r���[���W�n�̍��W���v�Z
*
* @param uv             �e�N�X�`�����W
* @param linearDepth    �[�x�l
*
* @return   �s�N�Z���̎��_���W
*/
vec3 GetViewSpacePosition(vec2 uv, float linearDepth)
{
    // �e�N�X�`�����W����NDC���W�ɕϊ�
    vec2 ndc = uv * 2 - 1;

    // NDC���W���王�_����̋���1m�̏ꍇ�̎��_���W�ɕϊ�
    /* ���_������`����1m�̈ʒu�ɃX�N���[����\�������ꍇ��uv���W */
    vec2 viewOneMeter = ndc * ndcToView;
    
    // �������[�x�l�̏ꍇ�̎��_���W�ɕϊ�
    return vec3(viewOneMeter * linearDepth, -linearDepth);
}


/**
* SAO(Scalable Ambient Obscurance)�ɂ��Օ��������߂�
*/
void main()
{
    // �s�N�Z���̎��_���W�Ɩ@�������߂�
    float depth = textureLod(texLinearDepth, inTexcoord, 0).x;
    vec3 positionVS = GetViewSpacePosition(inTexcoord, depth);
    /* X��������Y�������̎��_���W�̍������O�ς��邱�ƂŁA���ʂ̖@���x�N�g�����v�Z���� */
    vec3 normalVS = normalize(cross(dFdx(positionVS), dFdy(positionVS)));

    // �^���I�ȃ����_���l�𐶐����s���J�n�p�x�Ɏw��
    /* �t���O�����g���Ƃɉ�]�̊J�n�p�x�����炷���ƂŌ����ڂ����P���� */
    const ivec2 iuv = ivec2(gl_FragCoord.xy);
    const float startAngle = mod((3 * iuv.x ^ iuv.y + iuv.x * iuv.y) * 10, TwoPI);
    
    // ���[���h���W�n�ƃX�N���[�����W�n�̃T���v�����O���a
    float radiusTS = 
        SAO_Radius_in_Textures / depth;   // �e�N�X�`�����W�n�̃T���v�����O���a
    float pixelsSS = 
        radiusTS * textureSize(texLinearDepth, 0).y; // �X�N���[�����W�n�̃T���v�����O���a
    
    float occlusion = 0; // �Օ���
    for (int i = 0; i < SamplingCount; ++i) {
        // �T���v���_�̊p�x�Ƌ��������߂�
        float ratio = (float(i) + 0.5) * (1.0 / SamplingCount);
        float angle = ratio * (SpiralTurns * TwoPI) + startAngle;
        vec2 unitOffset = vec2(cos(angle), sin(angle)); 
        
        // �T���v���_�̎��_���W�����߂�
        vec2 uv = inTexcoord + ratio * radiusTS * unitOffset;
        
        // �����������قǍ����~�b�v���x����I��
        /* �ł��d�v�x�̍����r�b�g����~�b�v���x����I������ 
           �i��F1��MSB��0�A100��MSB��7�A10000��MSB��13�j
           �i�����l��15�ȉ��̏ꍇ�AMSB��logMipOffset�̒l3�ȉ��ɂȂ�̂ŁA
           �@����15�s�N�Z���ȉ��Ȃ�~�b�v���x��0�̉摜���g����j*/
        /* MSB���u�����܂��ȋ����������p�����[�^�v�Ƃ��Ďg���Ă���B
        �@ ���ۂɁAMSB��1�����邲�ƂɃs�N�Z�������͂��悻2�{�ɂȂ� */
        int mipLevel = clamp(findMSB(int(ratio * pixelsSS)) - LogMipOffset, 0, MaxMipLevel);
        
        // �T���v���_�̎��_���W�����߂�
        float sampleDepth = textureLod(texLinearDepth, uv, mipLevel).x;
        vec3 samplePositionVS = GetViewSpacePosition(uv, sampleDepth);

        // �T���v���_�ւ̃x�N�g���Ɩ@���̃R�T�C�������߂�
        /* �R�T�C����1(����)�ɋ߂��قǎՕ������オ�� */
        vec3 v = samplePositionVS - positionVS;
        float vn = dot(v, normalVS); // �p�x�ɂ��Օ���
        
        // �T���v���_�����S�ɋ߂��قǎՕ������オ��
        float vv = dot(v, v);
        float f = max(SAO_Radius2 - vv, 0); // �����ɂ��Օ���
        
        // �T���v���_�܂ł̋����ƃR�T�C������AO�����߂�
        occlusion += f * f * f * max((vn - SAO_Bias) / (vv + 0.001), 0);
    }
    
    // ���ϒl�����߁AAO�̋�������Z����
    occlusion = min(1.0, occlusion / SamplingCount * SAO_Intensity);

    // �����܂��͐����ɋ߂��ʂ̃I�N���[�W�����𕽋ω�����B
    /* �אڂ���s�N�Z���Ƃ̐[�x�l�̍������Ȃ��ꍇ�A
       �אڃs�N�Z���Ƃ̊ԂŎՕ����𕽋ω����� */
    /* �X�N���[�����W�������̂Ƃ��͍����̔��������Z�A
       ��̂Ƃ��͍����̔��������Z���� */
    if (abs(dFdx(positionVS.z)) < 0.02) {
        occlusion -= dFdx(occlusion) * ((iuv.x & 1) - 0.5);
    } 
    if (abs(dFdy(positionVS.z)) < 0.02) {
        occlusion -= dFdy(occlusion) * ((iuv.y & 1) - 0.5);
    }

    outColor = occlusion;
}
