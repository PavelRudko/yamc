#include "Renderer.h"

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "gl3w.h"

namespace yamc
{
	Renderer::Renderer() :
		fontTexture(FontBitmapPath),
		atlasTexture(AtlasPath),
		fontShader(FontVertexShaderPath, FontFragmentShaderPath),
		colorShader(ColorVertexShaderPath, ColorFragmentShaderPath),
		defaultShader(DefaultVertexShaderPath, DefaultFragmentShaderPath)
	{
		initFont();
		initOutlineMesh();
	}

	void Renderer::renderText(const glm::mat4& projectionMatrix, const std::string& text, const glm::vec3& color, const glm::vec2& offset, uint32_t scale) const
	{
		auto scaleMatrix = glm::scale(glm::mat4(1), glm::vec3(scale, scale, scale));
		glm::vec3 characterOffsetVector(offset.x, offset.y, 0);
	
		fontShader.use();
		int projectionMatrixLocation = glGetUniformLocation(fontShader.getID(), "mvp");
		int textureOffsetLocation = glGetUniformLocation(fontShader.getID(), "textureOffset");
		int colorLocation = glGetUniformLocation(fontShader.getID(), "color");

		glBindTexture(GL_TEXTURE_2D, fontTexture.getID());
		glBindVertexArray(fontCharacterMesh.getVAO());

		for (int i = 0; i < text.length(); i++) {
			auto textureOffset = getFontTextureOffset(text[i]);
			characterOffsetVector.x += (FontCharacterOffset + fontCharacterWidth) * scale;
			auto offsetMatrix = glm::translate(glm::mat4(1), characterOffsetVector);
			auto mvp = projectionMatrix * offsetMatrix * scaleMatrix;

			glUniformMatrix4fv(projectionMatrixLocation, 1, false, glm::value_ptr(mvp));
			glUniform2fv(textureOffsetLocation, 1, glm::value_ptr(textureOffset));
			glUniform3fv(colorLocation, 1, glm::value_ptr(color));
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		
		glBindVertexArray(0);
		glUseProgram(0);
	}

	void Renderer::renderCubeOutline(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& center) const
	{
		glUseProgram(colorShader.getID());
		int mvpLocation = glGetUniformLocation(colorShader.getID(), "mvp");
		int colorLocation = glGetUniformLocation(colorShader.getID(), "color");

		auto modelMatrix = glm::translate(glm::identity<glm::mat4>(), center);
		auto mvp = projectionMatrix * viewMatrix * modelMatrix;
		glm::vec4 outlineColor(0.6f, 0.6f, 0.6f, 1.0f);
		glUniformMatrix4fv(mvpLocation, 1, false, glm::value_ptr(mvp));
		glUniform4fv(colorLocation, 1, glm::value_ptr(outlineColor));
		glLineWidth(3);

		glBindVertexArray(outlineMesh.getVAO());
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(0);
	}

	void Renderer::renderCross(const glm::mat4& projectionMatrix, const glm::vec2& center) const
	{
		glUseProgram(colorShader.getID());
		int mvpLocation = glGetUniformLocation(colorShader.getID(), "mvp");
		int colorLocation = glGetUniformLocation(colorShader.getID(), "color");
		auto modelMatrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(center.x, center.y, 0));
		auto mvp = projectionMatrix * modelMatrix * glm::scale(glm::identity<glm::mat4>(), glm::vec3(26, 26, 26));
		glm::vec4 outlineColor(0.6f, 0.6f, 0.6f, 0.6f);
		glUniformMatrix4fv(mvpLocation, 1, false, glm::value_ptr(mvp));
		glUniform4fv(colorLocation, 1, glm::value_ptr(outlineColor));
		glLineWidth(4);

		glBindVertexArray(outlineMesh.getVAO());
		glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, (void*)(24 * sizeof(uint32_t)));
		glBindVertexArray(0);

		glUseProgram(0);
	}

	void Renderer::renderTerrain(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const Terrain& terrain) const
	{
		defaultShader.use();
		glBindTexture(GL_TEXTURE_2D, atlasTexture.getID());

		int mvpLocation = glGetUniformLocation(defaultShader.getID(), "mvp");

		for (const auto& pair : terrain.getChunks()) {
			uint64_t key = pair.first;
			auto chunkOffset = Terrain::getChunkOffset(key);

			glm::vec3 offset(chunkOffset.x * (float)Chunk::MaxWidth, 0, chunkOffset.y * (float)Chunk::MaxLength);

			auto chunk = pair.second;
			if (chunk->getIndicesCount() > 0) {
				auto modelMatrix = glm::translate(glm::identity<glm::mat4>(), offset);
				auto mvp = projectionMatrix * viewMatrix * modelMatrix;
				glUniformMatrix4fv(mvpLocation, 1, false, glm::value_ptr(mvp));

				glBindVertexArray(chunk->getVAO());
				glDrawElements(GL_TRIANGLES, chunk->getIndicesCount(), GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
		}

		glUseProgram(0);
	}

	void Renderer::initFont()
	{
		fontCharacterWidth = fontTexture.getWidth() / FontCharactersPerRow;
		fontCharacterHeight = fontTexture.getHeight() / FontRowCount;
		float characterUVHeight = fontCharacterHeight / (float)fontTexture.getHeight();
		float characterUVWidth = fontCharacterWidth / (float)fontTexture.getWidth();

		std::vector<glm::vec3> positions = {
			{0, 0, 0},
			{0, fontCharacterHeight, 0},
			{fontCharacterWidth, fontCharacterHeight, 0},
			{fontCharacterWidth, 0, 0}
		};

		std::vector<glm::vec2> uvs = {
			{0, 0},
			{0, characterUVHeight},
			{characterUVWidth, characterUVHeight},
			{characterUVWidth, 0}
		};

		std::vector<uint32_t> indices = {
			0, 2, 1, 0, 3, 2
		};

		fontCharacterMesh.setData(positions, uvs, indices);
	}

	void Renderer::initOutlineMesh()
	{
		float s = 0.5f;
		std::vector<glm::vec3> outlinePositions =
		{
			//cube
			{-s, -s, s},
			{-s, s, s},
			{-s, s, -s},
			{-s, -s, -s},
			{s, -s, s},
			{s, s, s},
			{s, s, -s},
			{s, -s, -s},

			//cross
			{0, -s, 0},
			{0, s, 0},
			{-s, 0, 0},
			{s, 0, 0}
		};

		std::vector<uint32_t> outlineIndices =
		{
			//cube: 0-23
			0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 3, 7, 2, 6,

			//cross: 24-27
			8, 9, 10, 11
		};

		outlineMesh.setData(outlinePositions, outlineIndices);
	}

	glm::vec2 Renderer::getFontTextureOffset(uint8_t character) const
	{
		uint32_t row = ((uint32_t)character) / FontCharactersPerRow;
		uint32_t column = ((uint32_t)character) % FontCharactersPerRow;
		return glm::vec2(column / (float)FontCharactersPerRow, row / (float)FontRowCount);
	}

	Renderer::~Renderer()
	{
	}
}