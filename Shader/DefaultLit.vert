/**
* @file DefaultLit.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition;  // ���_���W
layout(location=1) in vec2 inTexcoord;  // �e�N�X�`�����W
layout(location=2) in vec3 inNormal;    // �@���x�N�g��
layout(location=3) in vec4 inTangent;   // �^���W�F���g�x�N�g��

// �V�F�[�_����̏o��
layout(location=0) out vec3 outPosition;    // ���[���h���W
layout(location=1) out vec2 outTexcoord;    // �e�N�X�`�����W
layout(location=2) out vec3 outNormal;      // �@���x�N�g��
layout(location=3) out vec4 outTangent;     // �^���W�F���g�x�N�g��
layout(location=4) out vec3 outShadowTexcoord; // �V���h�E�e�N�X�`�����W

// �v���O��������̓���
layout(location=0) uniform mat4 transformMatrix;   // �`�惂�f���̍��W�ϊ��s��
layout(location=1) uniform mat4 cameraTransformMatrix;  // �J�����̍��W�ϊ��s��

// x: ����p�ɂ�鐅���g�嗦
// y: ����p�ɂ�鐂���g�嗦
// z: ���ߖ@�p�����[�^A
// w: ���ߖ@�p�����[�^B
layout(location=2) uniform vec4 cameraInfo;

layout(location=10) uniform mat4 shadowTextureMatrix; // �V���h�E�e�N�X�`���s��
layout(location=11) uniform float shadowNormalOffset; // ���W��@�������ɂ��炷��


/**
* �G���g���[�|�C���g
*/
void main()
{
    outTexcoord = inTexcoord;

    // �g��k���Ɖ�]�ƕ��s�ړ�
    gl_Position = transformMatrix * vec4(inPosition, 1);

    outPosition = gl_Position.xyz;

    // ���[���h�@�����v�Z�i�e���͉�]�̂݁j
    mat3 matNormal = transpose(inverse(mat3(transformMatrix)));
    /* 
     1.�u�g�嗦�̋t�����|������]�s��v�����ɂ́A
        inverse(�C���o�[�X)����transpose(�g�����X�|�[�Y)����B(�t�]�u�s��)

     2. �@���ɋt�]�u�s����|�������ʂ͒P�ʃx�N�g���ɂȂ�Ȃ�(���Ƃ�����)�̂ŁA
        ���K������K�v������B 
    */
    outNormal = normalize(matNormal * inNormal);

    // ���[���h�^���W�F���g���v�Z
    /* ���s�ړ����܂܂Ȃ�matNormal�s����g���ă��[���h���W�n�ɕϊ� */
    outTangent.xyz = normalize(matNormal * inTangent.xyz);  
    outTangent.w = inTangent.w;

    // �V���h�E�e�N�X�`�����W���v�Z
    outShadowTexcoord = outPosition + outNormal * shadowNormalOffset;
    outShadowTexcoord = vec3(shadowTextureMatrix * vec4(outShadowTexcoord, 1));

    // ���[���h���W�n����r���[���W�n�ɕϊ�
    vec3 pos = outPosition - vec3(cameraTransformMatrix[3]);
    gl_Position.xyz = pos * mat3(cameraTransformMatrix);
  
    // ����p�𔽉f
    gl_Position.xy *= cameraInfo.xy;
  
    // ���ߖ@��L���ɂ���
    gl_Position.w = -gl_Position.z;
  
    // �[�x�l��␳�i�[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ɂ���j
    gl_Position.z = -gl_Position.z * cameraInfo.w + cameraInfo.z; 
}