#include "GltfFile.h"

#include "../../UniformLocation.h"
#include "../Texture/Texture.h"
#include "../GraphicsObject//VertexArrayObject.h"

/**
* glTF���b�V����`�悷��
*
* @param mesh			�`�悷�郁�b�V��
* @param program		�`��v���O����
* @param objectColor	�Q�[���I�u�W�F�N�g�F
* @param materials		�}�e���A���z��
*/
void DrawGltf(
	const GltfMesh& mesh, GLuint program,
	const vec4& objectColor, const MaterialList& materials)
{
	bool hasUniformColor =	// �J���[���j�t�H�[���̗L��
		glGetUniformLocation(program, "color") >= 0;
	bool hasUniformEmission =	//�G�~�b�V�������j�t�H�[���̗L��
		glGetUniformLocation(program, "emission") >= 0;

	// ���ׂẴv���~�e�B�u��`��
	for (const auto& prim : mesh.primitives) {
		// �}�e���A����ݒ�
		if (prim.materialNo >= 0 && prim.materialNo < materials.size()) {
			const Material& material = *materials[prim.materialNo];

			if (hasUniformColor) {
				const vec4 color = objectColor * material.baseColor;
				glProgramUniform4fv(program, Loc::Color, 1, &color.x);
			}

			if (hasUniformEmission) {
				glProgramUniform1i(program, Loc::UseTexEmission,
					static_cast<bool>(material.texEmission));
				glProgramUniform4fv(program, Loc::Emission, 1, &material.emission.x);
			}

			// OpenGL�R���e�L�X�g�Ƀe�N�X�`�������蓖�Ă�
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

			if (material.texEmission) {
				const GLuint tex = *material.texEmission;
				glBindTextures(Bind::Texture::Emission, 1, &tex);
			}
			else {
				glBindTextures(Bind::Texture::Emission, 1, nullptr);
			}
		}

		glBindVertexArray(*prim.vao);
		//glDrawElements(prim.mode, prim.count, prim.type, prim.indices);
		glDrawElementsInstancedBaseVertex(prim.mode, prim.count,
			prim.type, prim.indices, 1, prim.baseVertex);
	}

	// OpenGL�R���e�L�X�g�ւ̊��蓖�Ă�����
	glBindVertexArray(0);
}