/**
* @ file StaticMesh.cpp
*/
#include "StaticMesh.h"

#include "../../UniformLocation.h"

/**
* ���b�V����`�悷��
*
* @param mesh			�`�悷�郁�b�V��
* @param program		�`��v���O����
* @param objectColor	�Q�[���I�u�W�F�N�g�F
* @param materials		�}�e���A���z��
*/
void DrawStaticMesh(
	const StaticMesh& mesh, GLuint program, 
	const vec4& objectColor, const MaterialList& materials)
{
	bool hasUniformColor = 	// �J���[���j�t�H�[���̗L��
		glGetUniformLocation(program, "color") >= 0;
	bool hasUniformEmission =	// �G�~�b�V�������j�t�H�[���̗L��
		glGetUniformLocation(program, "emission") >= 0;	
	bool hasUniformRoughnessAndMetallic =	// ���t�l�X�ƃ��^���b�N���j�t�H�[���̗L��
		glGetUniformLocation(program, "roughnessAndMetallic") >= 0;

	for (const auto& prim : mesh.primitives) {
		// �}�e���A����ݒ�
		if (prim.materialNo >= 0 && prim.materialNo < materials.size()) {
			const Material& material = *materials[prim.materialNo];

			if (hasUniformColor) {
				const vec4 color = objectColor * material.baseColor;
				glProgramUniform4fv(program, Loc::Color, 1, &color.x);
			}
			if (hasUniformEmission) {
				// �G�~�b�V������ݒ�
				glProgramUniform1i(program, Loc::UseTexEmission,
					static_cast<bool>(material.texEmission));

				glProgramUniform4fv(program, Loc::Emission, 1, &material.emission.x);
			}
			if (hasUniformRoughnessAndMetallic) {
				// ���t�l�X�ƃ��^���b�N��ݒ�
				glProgramUniform2f(program, Loc::RoughnessAndMetallic,
					material.roughness, material.metallic);
			}
		
			// GL�R���e�L�X�g�Ƀe�N�X�`�������蓖�Ă�
			if (material.texBaseColor) {
				const GLuint tex = *material.texBaseColor;
				glBindTextures(Bind::Texture::Color, 1, &tex);
			}

			if (material.texNormal) {
				const GLuint tex = *material.texNormal;
				glBindTextures(Bind::Texture::Normal, 1, &tex);
			}
			else {
				glBindTextures(Bind::Texture::Normal, 1, nullptr);
			}

			if (material.texEmission && hasUniformEmission) {
				const GLuint tex = *material.texEmission;
				glBindTextures(Bind::Texture::Emission, 1, &tex);
			}
			else {
				// �G�~�b�V�����e�N�X�`���𖢐ݒ�ɂ���(�ݒ肳�ꂽ�܂܂��ƌ������ቺ����)
				glBindTextures(Bind::Texture::Emission, 1, nullptr); 
			}

		}
		DrawPrimitive(prim);
	}
}

/**
* �X�v���C�g��`�悷��
*
* @param mesh			�X�v���C�g���b�V��
* @param program		�`��v���O����
* @param objectColor	�Q�[���I�u�W�F�N�g�F
* @param materials		�}�e���A���z��
*/
void DrawSpritePlaneMesh(
	const StaticMesh& planeMesh, GLuint program, 
	const vec4& objectColor, const Material& material)
{
	bool hasUniformColor = 	// �J���[���j�t�H�[���̗L��
		glGetUniformLocation(program, "color") >= 0;
	bool hasUniformEmission =	//�G�~�b�V�������j�t�H�[���̗L��
		glGetUniformLocation(program, "emission") >= 0;

	if (hasUniformColor) {
		const vec4 color = objectColor * material.baseColor;
		glProgramUniform4fv(program, Loc::Color, 1, &color.x);
	}

	if (hasUniformEmission) {
		glProgramUniform1i(program, Loc::UseTexEmission,
			static_cast<bool>(material.texEmission));

		glProgramUniform4fv(program, Loc::Emission, 1, &material.emission.x);
	}

	// GL�R���e�L�X�g�Ƀe�N�X�`�������蓖�Ă�
	if (material.texBaseColor) {
		const GLuint tex = *material.texBaseColor;
		glBindTextures(Bind::Texture::Color, 1, &tex);
	}

	if (material.texEmission) {
		const GLuint tex = *material.texEmission;
		glBindTextures(Bind::Texture::Emission, 1, &tex);
	}
	else {
		// �G�~�b�V�����e�N�X�`���𖢐ݒ�ɂ���(�ݒ肳�ꂽ�܂܂��ƌ������ቺ����)
		glBindTextures(Bind::Texture::Emission, 1, nullptr);
	}

	// �v���~�e�B�u��`�悷��
	for (const auto& prim : planeMesh.primitives) {
		DrawPrimitive(prim);
	}
}
