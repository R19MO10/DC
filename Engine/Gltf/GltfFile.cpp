#include "GltfFile.h"

#include "../../UniformLocation.h"
#include "../Texture/Texture.h"
#include "../GraphicsObject//VertexArrayObject.h"

/**
* glTFメッシュを描画する
*
* @param mesh			描画するメッシュ
* @param program		描画プログラム
* @param objectColor	ゲームオブジェクト色
* @param materials		マテリアル配列
*/
void DrawGltf(
	const GltfMesh& mesh, GLuint program,
	const vec4& objectColor, const MaterialList& materials)
{
	bool hasUniformColor =	// カラーユニフォームの有無
		glGetUniformLocation(program, "color") >= 0;
	bool hasUniformEmission =	//エミッションユニフォームの有無
		glGetUniformLocation(program, "emission") >= 0;

	// すべてのプリミティブを描画
	for (const auto& prim : mesh.primitives) {
		// マテリアルを設定
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

			// OpenGLコンテキストにテクスチャを割り当てる
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

	// OpenGLコンテキストへの割り当てを解除
	glBindVertexArray(0);
}