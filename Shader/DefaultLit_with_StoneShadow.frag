/**
* @file DefaultLit_with_StoneShadow.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition;	// ���[���h���W
layout(location=1) in vec2 inTexcoord;	// �e�N�X�`�����W
layout(location=2) in vec3 inNormal;	// �@���x�N�g��
layout(location=3) in vec4 inTangent;	// �^���W�F���g�x�N�g��
layout(location=4) in vec3 inShadowTexcoord; // �V���h�E�e�N�X�`�����W

// �o�͂���F�f�[�^
out vec4 outColor;

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texColor;	// ��{�F�e�N�X�`��
layout(binding=1) uniform sampler2D texEmission;// �����F�e�N�X�`��
layout(binding=2) uniform sampler2D texNormal;	// �@���e�N�X�`��
layout(binding=3) uniform sampler2DShadow texShadow; // �e�p�̐[�x�e�N�X�`��

// �v���O��������̓���
layout(location=1) uniform mat4 cameraTransformMatrix;  // �J�����̍��W�ϊ��s��

layout(location=99)  uniform float alphaCutoff; // �t���O�����g��j�����鋫�E�l
layout(location=100) uniform vec4 color;		// ���̂̐F
layout(location=101) uniform int useTexEmission;// �G�~�b�V�����e�N�X�`���̗L��
layout(location=102) uniform vec4 emission;		// ���̂̔����F

layout(location=107) uniform vec3 ambientLight;	// ����

// ���s����
struct DirectionalLight
{
	vec3 color;     // �F�Ɩ��邳
	vec3 direction; // ���̌���
};
layout(location=108) uniform DirectionalLight directionalLight;

// ���C�g
struct Light
{
	vec3 color;		// �F�Ɩ��邳
	float falloffAngle; // �X�|�b�g���C�g�̌����J�n�p�x(Spot)
	
	vec3 position;	// �ʒu
	float radius;	// ���C�g���͂��ő唼�a

	vec3 direction; // ���C�g�̌���(Spot)
	float coneAngle;// �X�|�b�g���C�g���Ƃ炷�p�x(Spot)
};
// ���C�g�pSSBO
layout(std430, binding=0) readonly buffer LightDataBlock
{
	int lightCount;	// ���C�g�̐�
	int dummy[3];	// ��L��lightCount�ƍ��킹��64�o�C�g�ɂ��邽�߂̖��ߍ��킹
	Light lightList[];	// ���ׂẴ��C�g�f�[�^
};

// �e���쐬���闎���΂̍ő吔
#define MAX_FALLSTONE 16

// �����΂̐�
layout(location=120) uniform int fallStoneCount;

// xy: XZ���ʏ�̍��W
// z:  �΂̔��a
// w:  �e�̔Z�x
layout(location=121) uniform vec4 fallStoneInfo[MAX_FALLSTONE];


const float PI = 3.14159265;	// ��
const float Gamma = 2.2;		// �K���}�l(2.2��CRT���j�^�[�̃K���}�l)
const float MaxFallStoneShadowIntensity = 0.8;	// �����΂̉e�̔Z�x�̍ő�l
const float FallStoneShadowFadeStartRatio = 0.8;// �����΂̉e�̃t�F�[�h���n�߂钆�S����̊���


/**
* �@�����v�Z����
*/
vec3 ComputeWorldNormal()
{
	vec3 normal = texture(texNormal, inTexcoord).rgb;
	
	/* �l������������ꍇ�A�@���e�N�X�`�����ݒ肳��Ă��Ȃ��Ɣ��f����
		���_�@����Ԃ�(RGB���S�� 0 �͂��肦�Ȃ�����) */
	if (dot(normal, normal) <= 0.0001) {
		return normalize(inNormal);
	}
	
	// 8bit�l�ł��邱�Ƃ��l������0�`1��-1�`+1�ɕϊ�(128��0�Ƃ݂Ȃ�)
	normal = normal * (255.0 / 127.0) - (128.0 / 127.0);

	/* �@���e�N�X�`���̒l�����[���h��Ԃɕϊ�
		(�o�C�^���W�F���g�̐�����������inTangent.w�ɓ���Ă���) */
	vec3 bitangent = inTangent.w * cross(inNormal, inTangent.xyz);
	
	return normalize(
		inTangent.xyz * normal.x + bitangent * normal.y + inNormal * normal.z);
}


/**
* �G���g���[�|�C���g
*/
void main()
{
	vec4 c = texture(texColor, inTexcoord);

	// �J�b�g�I�t�l���w�肳��Ă���ꍇ�A�A���t�@�����̒l�����̏ꍇ�͕`����L�����Z��
	if (alphaCutoff > 0) {
		if (c.a < alphaCutoff) {
			discard; // �t���O�����g��j��(�L�����Z��)
		}
		c.a = 1; // �j������Ȃ�������s�����ɂ���
    }

	// �e�N�X�`���̃K���}�␳������
	c.rgb = pow(c.rgb, vec3(Gamma));

	outColor = c * color;
	
	// ���`��Ԃɂ���Ē�����1�ł͂Ȃ��Ȃ��Ă���̂ŁA���K�����Ē�����1�ɕ�������
	/* ���_�V�F�[�_�[�ƃt���O�����g�V�F�[�_�[�ԂŒl���ς���Ă��܂����߂�����x���K��	
		(���`�ۊǂ���Ē������P����Ȃ��Ȃ邩��) */
	vec3 normal = ComputeWorldNormal();	

	// �����̕���
	vec3 diffuse = vec3(0); // �g�U���̖��邳�̍��v

	// �����̏������s��
	for (int i = 0; i < lightCount; ++i) {
		vec3 direction = lightList[i].position - inPosition;
	
		// �����܂ł̋���
		float sqrDistance = dot(direction, direction);	/* ���ςɂ���ċ����̂Q������߂� */
		float distance = sqrt(sqrDistance);
  	
		// �����𐳋K�����Ē�����1�ɂ���
		direction = normalize(direction);
    
		// �����x���g�̗]�������g���Ė��邳���v�Z
		/* a,b�̒������P�ɂ��Ă�����cos�� = a�Eb�ɂ��鎖���ł��� */
		/* ���邳���}�C�i�X�ɂȂ�Ȃ��悤�ɍŏ��l��0�ɂ��Ă��� */
		float theta = max(dot(direction, normal), 0);
  	
		// �����o�[�g���˂ɂ�锽�ˌ��̃G�l���M�[�ʂ���ˌ��Ɠ��������邽�߂Ƀ΂Ŋ���
		float illuminance = theta / PI;
  	
		// �Ǝˊp�x��0���傫����΃X�|�b�g���C�g�Ƃ݂Ȃ�
		if (lightList[i].coneAngle > 0) {
			// ���C�g����t���O�����g�֌������x�N�g�� �� �X�|�b�g���C�g�̃x�N�g�� �̂Ȃ��p���v�Z
			// �p�x��coneAngle�ȏ�Ȃ�͈͊O
			float angle = acos(dot(-direction, lightList[i].direction));
	
			if (angle >= lightList[i].coneAngle) {
				continue; // �Ǝ˔͈͊O
			}
	
			// �ő�Ǝˊp�x�̂Ƃ�0, �����J�n�p�x�̂Ƃ�1�ɂȂ�悤�ɕ��
		    float a = 
				min((lightList[i].coneAngle - angle) / (lightList[i].coneAngle - lightList[i].falloffAngle), 1);
		    illuminance *= a;
		} // if coneAngle
	
		// ���C�g�̍ő勗���𐧌�
		float radius = lightList[i].radius;
		float smoothFactor = clamp(1 - pow(distance / radius, 4), 0, 1);
		illuminance *= smoothFactor * smoothFactor;	
	
		// �t2��̖@���ɂ���Ė��邳������������
		/* 0���Z���N���Ȃ��悤��1�𑫂��Ă���
			(�������O�̎��Ɏw�肵�����邳�̍ő�l���o�邱�Ƃ��ۏႳ��Ă���) */
		illuminance /= sqrDistance + 1;
	
		// �g�U���̖��邳�����Z
		diffuse += lightList[i].color * illuminance;
	} // for lightCount

	// �e���v�Z
	float shadow = texture(texShadow, inShadowTexcoord).r;

	// ���s�����̖��邳���v�Z
	/*�u�t���O�����g���猩�����̌����v���K�v�Ȃ̂ŁA�������t�ɂ��� */
	float theta = max(dot(-directionalLight.direction, normal), 0);
	float illuminance = theta / PI;
	diffuse += directionalLight.color * illuminance * shadow;

	// �A���r�G���g���C�g�̖��邳���v�Z
	diffuse += ambientLight;

    // �g�U���̉e���𔽉f
	outColor.rgb *= diffuse;

	// �����΂̉e�̏������s��
	float fallStoneShadow = 0;	// �����΂̉e
	for (int i = 0; i < fallStoneCount; ++i) {
		 // ���ꂼ���XZ���W
		vec2 stonePosXZ = fallStoneInfo[i].xy;

		// �t���O�����g�Ɛ΂̋������v�Z
		float distance = distance(inPosition.xz, stonePosXZ);

		float radius = fallStoneInfo[i].z;	// �΂̔��a
		float shadowAlpha = fallStoneInfo[i].w;	// �e�̔Z�x

		// �e�̃t�F�[�h�������v�Z
		float ratio = smoothstep(radius * FallStoneShadowFadeStartRatio, radius, distance);
		
		// �t�F�[�h�������� MaxFallStoneShadowIntensity �� 0 �̊ԂŐ��`��Ԃ����Z
		fallStoneShadow += mix(MaxFallStoneShadowIntensity, 0, ratio) * shadowAlpha;

		// �e�̔Z�x�𐧌�
  		fallStoneShadow = min(fallStoneShadow, MaxFallStoneShadowIntensity);
	} // for fallStoneCount
	
	// �����΂̉e�𔽉f
	outColor.rgb *= 1 - fallStoneShadow;

	// �����F�𔽉f
	outColor.rgb += 
		(useTexEmission == 1 ? // �G�~�b�V�����e�N�X�`���L��
			texture(texEmission, inTexcoord).rgb * emission.rgb : emission.rgb);

	// �K���}�␳���s��
	outColor.rgb = pow(outColor.rgb, vec3(1 / Gamma));

	//�J������Z���W�𐳋K�����āA[-1, 1]�͈̔͂ɕϊ�
	float zNormalized = (gl_FragCoord.z * 2.0) - 1.0;
	//���K�����ꂽZ���W�����̉��߃N���b�v�ʂ͈̔͂ɖ߂�
	gl_FragDepth = (gl_DepthRange.diff * zNormalized + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}