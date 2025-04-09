/**
* @file Sprcular.frag
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
layout(location=103) uniform vec2 roughnessAndMetallic;
#define Roughness	(roughnessAndMetallic.x) // ���̕\�ʂ̑e��(0.0�`1.0)
#define Metallic	(roughnessAndMetallic.y) // �������ǂ���(0=����� 1=����)

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


const float PI = 3.14159265;		// ��
const float Gamma = 2.2;			// �K���}�l(2.2��CRT���j�^�[�̃K���}�l)


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
* �@�����z��
* 
* @param normal		�@��
* @param halfway	���ˌ����x�N�g���Ǝ����x�N�g���̒��ԃx�N�g��
* @param roughness	���̕\�ʂ̑e��
*
* @return ���_���猩���邷�ׂẴ}�C�N���t�@�Z�b�g�@���̕��ϒl
*/
float NormalDistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
	// ���������������ɔ��˂���������ʂ̔䗦�����߂�
	/* �\�ʖ@���̌������n�[�t�x�N�g���Ƌ߂��قǁA
		���������������ɔ��˂���������ʂ������Ȃ� */
	float NdotH = max(dot(normal, halfway), 0);
	float NdotH2 = NdotH * NdotH;

	// dot(N, H)���e������䗦�����߂�
	float r2 = roughness * roughness;
	float r4 = r2 * r2;
	float denom = (NdotH2 * (r4 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return r4 / denom;
}

/**
* �􉽌�����
* 
* @param NdotL		�@���Ɠ��ˌ����̌��������x�N�g���̓���
* @param NdotV		�@���Ǝ����x�N�g���̓���
* @param roughness	���̕\�ʂ̑e��
*
* @return ���������_�ɓ��B����䗦
*/
float GeometricAttenuationSchlick(float NdotL, float NdotV, float roughness)
{
	// ���̕\�ʂ̉��ʂ̓x����
	float k = (roughness + 1) * (roughness + 1) * 0.125;

	// �����������猩���􉽊w�I���������v�Z
	/* ���˃x�N�g���ɑ΂���������ʂ̌����₷�� */
	float g0 = NdotL / (NdotL * (1 - k) + k);

	// ���_�������猩���􉽊w�I���������v�Z
	/* �����x�N�g���ɑ΂���������ʂ̌����₷�� */
	float g1 = NdotV / (NdotV * (1 - k) + k);

	return g0 * g1;
}

/**
* �t���l����
* 
* @param fresnel0	�p�x0�̃t���l���l
* @param VdotH		���ˌ����ƃJ�����̎����̒��ԕ����x�N�g���̓���
*
* @return ���̔��˂Ɠ��߂̔䗦�x�N�g��(�t���l����)
*/
vec3 FresnelSchlick(vec3 fresnel0, float VdotH)
{
	// �V�����b�N�̋ߎ���
	/* ���˃x�N�g���Ɩ@���̂Ȃ��p��0���ɋ߂��قǔ��˂��キ�Ȃ�A
		90���ɋ߂��قǔ��˂������Ȃ� */
	float vh1 = 1 - VdotH;
	float vh2 = vh1 * vh1;
	return fresnel0 + (1 - fresnel0) * (vh2 * vh2 * vh1);
}


const float MinCosTheta = 0.000001;	// cos�Ƃ̍ŏ��l

/**
* ���ʔ���BRDF
*
* @param normal			�@��
* @param halfway		���ˌ����x�N�g���Ǝ����x�N�g���̒��ԃx�N�g��
* @param roughness		���̕\�ʂ̑e��
* @param cameraVector	�J�����̎��������x�N�g��
* @param NdotL			�@���Ɠ��ˌ����̌��������x�N�g���̓���
* @param fresnel		�t���l����
*
* @return ���ʔ��˂̋����x�N�g��
*/
vec3 SpecularBRDF(
  vec3 normal, vec3 halfway, float roughness, vec3 cameraVector, float NdotL, vec3 fresnel)
{
  // �@�����z�����v�Z
  float D = NormalDistributionGGX(normal, halfway, roughness);

  // �􉽊w�I���������v�Z
  float NdotV = max(dot(normal, cameraVector), MinCosTheta); /* �@���Ǝ����x�N�g���̓��� */
  float G = GeometricAttenuationSchlick(NdotL, NdotV, roughness);

  // ���ʔ���BRDF���v�Z
  float denom = 4 * NdotL * NdotV;
  return (D * G * fresnel) * (1 / denom);
}


// CalcBRDF�̌v�Z����
struct BRDFResult
{
	vec3 diffuse;  // �g�U����
	vec3 specular; // ���ʔ���
};
/**
* ���ʔ���BRDF�Ɗg�U����BRDF���܂Ƃ߂Čv�Z����
*
* @param normal			�@��
* @param fresnel0		�p�x0�̃t���l���l
* @param cameraVector	�J�����̎��������x�N�g��
* @param direction		���ˌ����̕����x�N�g��
* @param color			���ˌ����̐F
*
* @return ���ʔ��˂Ɗg�U���˂̋���
*/
BRDFResult CalcBRDF(vec3 normal, vec3 fresnel0,
  vec3 cameraVector, vec3 direction, vec3 color)
{
  // �t���l�����v�Z
  vec3 H = normalize(direction + cameraVector);
  vec3 F = FresnelSchlick(fresnel0, max(dot(cameraVector, H), 0));

  // GGX�ŋ��ʔ��˂��v�Z
  float NdotL = max(dot(normal, direction), MinCosTheta); /* �@���Ɠ��ˌ����̌��������x�N�g���̓��� */
  vec3 specularBRDF = SpecularBRDF(
	normal, H, Roughness, cameraVector, NdotL, F);
  vec3 specular = color * specularBRDF * NdotL;

  // ���K�������o�[�g�Ŋg�U���˂��v�Z
  float diffuseBRDF = NdotL / PI;
  /* �p�x90���ł�F��1.0�ɁA(1 - F)��0�ɂȂ�
	���p�x90���ł͊g�U���˂͋N�����A���ˌ���100%���ʔ��� */
  /* ���^���b�N��1(�����}�e���A��)�̂Ƃ��A�g�U���˂������������
	���u�����͊g�U���˂��N�����Ȃ��v�Ƃ����������ۂ��Č����邽�� */
  vec3 diffuse = color * diffuseBRDF * (1 - F) * (1 - Metallic);
 
  return BRDFResult(diffuse, specular);
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

	// �����x�N�g��
	vec3 cameraPos = vec3(cameraTransformMatrix[3]);
	vec3 cameraVector = normalize(cameraPos - inPosition);
	
	// �p�x0�̃t���l���l
	/* �������@���ƕ��s�Ȋp�x(��=0)�œ��˂����ꍇ�̋��ʔ��ˌW�� */
	vec3 fresnel0 = mix(vec3(0.04), outColor.rgb, Metallic);
	
	vec3 specular = vec3(0); // ���ʔ��ˌ��̖��邳�̍��v

	// �����̏������s��
	for (int i = 0; i < lightCount; ++i) {
		vec3 direction = lightList[i].position - inPosition;
	
		// �����܂ł̋���
		float sqrDistance = dot(direction, direction);	/* ���ςɂ���ċ����̂Q������߂� */
		float distance = sqrt(sqrDistance);
  	
		// �����𐳋K�����Ē�����1�ɂ���
		direction = normalize(direction);
    
		float illuminance = 1;

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
		const float radius = lightList[i].radius;
		const float smoothFactor = clamp(1 - pow(distance / radius, 4), 0, 1);
		illuminance *= smoothFactor * smoothFactor;	
		
		// �t2��̖@���ɂ���Ė��邳������������
		/* 0���Z���N���Ȃ��悤��1�𑫂��Ă���
			(�������O�̎��Ɏw�肵�����邳�̍ő�l���o�邱�Ƃ��ۏႳ��Ă���) */
		illuminance /= sqrDistance + 1;
		
		// �g�U���˂Ƌ��ʔ��˂��v�Z
		vec3 lightColor = lightList[i].color * illuminance;
	    BRDFResult result = CalcBRDF(normal, fresnel0, cameraVector, direction, lightColor);
	    diffuse += result.diffuse;
	    specular += result.specular;
	} // for lightCount

	// �e���v�Z
	float shadow = texture(texShadow, inShadowTexcoord).r;

	// ���s�����̊g�U���˂Ƌ��ʔ��˂��v�Z
	/*�u�t���O�����g���猩�����̌����v���K�v�Ȃ̂ŁA�������t�ɂ���*/
	BRDFResult result = CalcBRDF(normal, fresnel0, cameraVector,
	  -directionalLight.direction, directionalLight.color);
	specular += result.specular * shadow;
	diffuse += result.diffuse * shadow;

	// �A���r�G���g���C�g�̖��邳���v�Z
	/* �A���r�G���g���C�g�́u���ׂĂ̕�������̌��̕��ϒl�v�Ȃ̂ŁA
		�t���l���l���u���̂�����Ƃ��̕��ϓI�Ȋp�x�v��I�ԕK�v������ */
	/* ���̂𐳖ʂ��猩���Ƃ��A�p�x45�����E�̓����ƊO���̌������̖ʐς͓������Ȃ� */
	vec3 f45 = fresnel0 + (1 - fresnel0) * 0.0021555; // �p�x45���̃t���l���l
	specular += ambientLight * f45;
	diffuse += ambientLight * (1 - f45) * (1 - Metallic);

    // �g�U���̉e���𔽉f
	outColor.rgb *= diffuse;

	// ���ʔ��˂̉e���𔽉f
	outColor.rgb += specular;

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