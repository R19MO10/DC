/**
* @file Particle.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition;  // ���_���W
layout(location=1) in vec2 inTexcoord;  // �e�N�X�`�����W

// �V�F�[�_����̏o��
layout(location=0) out vec4 outColor;   // ��{�F
layout(location=1) out vec2 outTexcoord;// �e�N�X�`�����W

// �v���O��������̓���
layout(location=1) uniform mat4 cameraTransformMatrix;  // �J�����̍��W�ϊ��s��

// x: ����p�ɂ�鐅���g�嗦
// y: ����p�ɂ�鐂���g�嗦
// z: ���ߖ@�p�����[�^A
// w: ���ߖ@�p�����[�^B
layout(location=2) uniform vec4 cameraInfo;

// �p�[�e�B�N��
struct ParticleData
{
	vec3 position;	// ���W
	float rotation;	// ��]
	
	vec4 color;		// �F�ƕs�����x

	vec4 direction;	// xyz: ����, w: �����g�p�̗L��(0, 1)

	vec2 size;		// �g��k��
	float damy[2];  // 64�o�C�g��؂�ɂ��邽�߂̖��ߍ��킹
};
// �p�[�e�B�N���pSSBO
layout(std430, binding=2) buffer particleDataBlock
{
    ParticleData particleList[];
};


/**
* �G���g���[�|�C���g
*/
void main()
{
    // �p�[�e�B�N���̐F��ݒ�
    ParticleData particle = particleList[gl_InstanceID];
    outColor = particle.color;

    // �v���~�e�B�u�̃e�N�X�`�����W���v�Z
    outTexcoord = inTexcoord;

    // �p�[�e�B�N���̉�]�s����쐬
    mat3 rot = mat3(1);
    const float c = cos(particle.rotation);
    const float s = sin(particle.rotation);
    rot[0][0] = c;
    rot[0][1] = s;
    rot[1][0] = -s;
    rot[1][1] = c;
    rot[0] *= particle.size.x;
    rot[1] *= particle.size.y;

    // �J�����̍��W�ϊ��s�񂩂��]�s����擾
    const mat3 cameraRotation = mat3(cameraTransformMatrix);

    if (particle.direction.w == 0){
        // �������g�p���Ȃ��ꍇ�i�r���{�[�h�j
        rot = cameraRotation * rot; 
    }
    else {
        // �������g�p����ꍇ�i�����x�N�g���⒆�S����̃x�N�g���j

        // �I�C���[�p�𒊏o
        const float yaw =   // Y��
		    atan(cameraRotation[2][0], cameraRotation[2][2]);

	    const float sinY = sin(yaw);
	    const float cosY = cos(yaw);
	    const float roll = // Z��
		    atan(sinY * cameraRotation[1][2] - cosY * cameraRotation[1][0],
			    cosY * cameraRotation[0][0] - sinY * cameraRotation[0][2]);

        const float sinZ = sin(roll);
        const float cosZ = cos(roll);
	    const mat3 mz = {
	    	{ cosZ,		sinZ,		0	  },
	    	{ -sinZ,	cosZ,		0	  },
	    	{ 0,		0,			1	  }
        };
        
        const vec3 direction = normalize(vec3(particle.direction));
                       
        // y���Ƃ̊O�ς����Ax�������߂�
        const vec3 xAxis = normalize(cross(direction, vec3(0.0, 1.0, 0.0))); 
        // �����x�N�g����x���̊O�ς����Ay�������߂�
        const vec3 yAxis = normalize(cross(xAxis, direction)); 
        // z���͕����x�N�g���Ɠ���
        const vec3 zAxis = direction;
        
        mat3 rotateToDirection = mat3(xAxis, yAxis, zAxis);
        
        // X����90�x��]������p�̉�]�s��
        const mat3 rotateY90 = mat3(
            vec3(0.0, 0.0, -1.0),
            vec3(0.0, 1.0,  0.0),
            vec3(1.0, 0.0,  0.0)
        );
        rotateToDirection = rotateToDirection * mz * rotateY90;
       
        rot = rotateToDirection * rot;
    }
  
    const mat4 transMat = mat4(
        vec4(rot[0], 0.0), 
        vec4(rot[1], 0.0), 
        vec4(rot[2], 0.0), 
        vec4(particle.position, 1)
    );

    gl_Position = transMat * vec4(inPosition, 1);

    // ���[���h���W�n����r���[���W�n�ɕϊ�
    vec3 pos = gl_Position.xyz - vec3(cameraTransformMatrix[3]);
    gl_Position.xyz = pos * mat3(cameraTransformMatrix);
  
    // ����p�𔽉f
    gl_Position.xy *= cameraInfo.xy;
  
    // ���ߖ@��L���ɂ���
    gl_Position.w = -gl_Position.z;
  
    // �[�x�l��␳�i�[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ɂ���j
    gl_Position.z = -gl_Position.z * cameraInfo.w + cameraInfo.z;
}