/**
* @ file StaticMesh.cpp
*/
#include "StaticMesh.h"

#include "../../UniformLocation.h"

/**
* メッシュを描画する
*
* @param mesh			描画するメッシュ
* @param program		描画プログラム
* @param objectColor	ゲームオブジェクト色
* @param materials		マテリアル配列
*/
void DrawStaticMesh(
	const StaticMesh& mesh, GLuint program, 
	const vec4& objectColor, const MaterialList& materials)
{
	bool hasUniformColor = 	// カラーユニフォームの有無
		glGetUniformLocation(program, "color") >= 0;
	bool hasUniformEmission =	// エミッションユニフォームの有無
		glGetUniformLocation(program, "emission") >= 0;	
	bool hasUniformRoughnessAndMetallic =	// ラフネスとメタリックユニフォームの有無
		glGetUniformLocation(program, "roughnessAndMetallic") >= 0;

	for (const auto& prim : mesh.primitives) {
		// マテリアルを設定
		if (prim.materialNo >= 0 && prim.materialNo < materials.size()) {
			const Material& material = *materials[prim.materialNo];

			if (hasUniformColor) {
				const vec4 color = objectColor * material.baseColor;
				glProgramUniform4fv(program, Loc::Color, 1, &color.x);
			}
			if (hasUniformEmission) {
				// エミッションを設定
				glProgramUniform1i(program, Loc::UseTexEmission,
					static_cast<bool>(material.texEmission));

				glProgramUniform4fv(program, Loc::Emission, 1, &material.emission.x);
			}
			if (hasUniformRoughnessAndMetallic) {
				// ラフネスとメタリックを設定
				glProgramUniform2f(program, Loc::RoughnessAndMetallic,
					material.roughness, material.metallic);
			}
		
			// GLコンテキストにテクスチャを割り当てる
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
				// エミッションテクスチャを未設定にする(設定されたままだと効率が低下する)
				glBindTextures(Bind::Texture::Emission, 1, nullptr); 
			}

		}
		DrawPrimitive(prim);
	}
}

/**
* スプライトを描画する
*
* @param mesh			スプライトメッシュ
* @param program		描画プログラム
* @param objectColor	ゲームオブジェクト色
* @param materials		マテリアル配列
*/
void DrawSpritePlaneMesh(
	const StaticMesh& planeMesh, GLuint program, 
	const vec4& objectColor, const Material& material)
{
	bool hasUniformColor = 	// カラーユニフォームの有無
		glGetUniformLocation(program, "color") >= 0;
	bool hasUniformEmission =	//エミッションユニフォームの有無
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

	// GLコンテキストにテクスチャを割り当てる
	if (material.texBaseColor) {
		const GLuint tex = *material.texBaseColor;
		glBindTextures(Bind::Texture::Color, 1, &tex);
	}

	if (material.texEmission) {
		const GLuint tex = *material.texEmission;
		glBindTextures(Bind::Texture::Emission, 1, &tex);
	}
	else {
		// エミッションテクスチャを未設定にする(設定されたままだと効率が低下する)
		glBindTextures(Bind::Texture::Emission, 1, nullptr);
	}

	// プリミティブを描画する
	for (const auto& prim : planeMesh.primitives) {
		DrawPrimitive(prim);
	}
}
