/**
* @file Sao.frag
*/
#version 450

layout(location=1) in vec2 inTexcoord;

// �o�͂���F�f�[�^
out vec4 outColor;

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texShield;	// SAO�e�N�X�`���i�Օ����j
layout(binding=1) uniform sampler2D texColor;	// SAO��K������X�N���[���e�N�X�`��


/**
* SAO���Q�[����ʂɓK������
*/
void main()
{
	// �Օ������擾
	float occlusion = texture(texShield, inTexcoord).x;
	
	outColor = texture(texColor, inTexcoord);
	outColor = vec4(vec3(outColor) * (1 - occlusion), outColor.w);
}