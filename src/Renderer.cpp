#include "Renderer.h"

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "gl3w.h"

namespace yamc
{
	Renderer::Renderer() :
		fontTexture(FontBitmapPath, FontCharactersPerRow, FontRowCount),
		atlasTexture(AtlasPath, AtlasTilesPerRow, AtlasRowCount),
		uiShader(UIVertexShaderPath, UIFragmentShaderPath),
		colorShader(ColorVertexShaderPath, ColorFragmentShaderPath),
		defaultShader(DefaultVertexShaderPath, DefaultFragmentShaderPath)
	{
		initUIMesh();
		initOutlineMesh();
		initBlockMesh();
	}

	void Renderer::renderText(const glm::mat4& projectionMatrix, const std::string& text, const glm::vec3& color, const glm::vec2& offset, uint32_t scale) const
	{
		auto scaleMatrix = glm::scale(glm::mat4(1), glm::vec3(scale, scale, scale));
		glm::vec3 characterOffsetVector(offset.x, offset.y, 0);
	
		uiShader.use();
		int projectionMatrixLocation = glGetUniformLocation(uiShader.getID(), "mvp");
		int textureOffsetLocation = glGetUniformLocation(uiShader.getID(), "textureOffset");
		int colorLocation = glGetUniformLocation(uiShader.getID(), "color");

		glBindTexture(GL_TEXTURE_2D, fontTexture.getID());
		glBindVertexArray(uiMesh.getVAO());

		for (int i = 0; i < text.length(); i++) {
			auto textureOffset = fontTexture.getTileOffset(text[i]);
			characterOffsetVector.x += (FontCharacterOffset + fontTexture.getTileWidth()) * scale;
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
		int textureOffsetLocation = glGetUniformLocation(defaultShader.getID(), "textureOffset");

		glUniform2f(textureOffsetLocation, 0, 0);

		for (const auto& pair : terrain.getChunks()) {
			uint64_t key = pair.first;
			auto chunkOffset = getChunkOffset(key);

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

	void Renderer::renderInventoryHotbar(const glm::mat4& projectionMatrix, const glm::vec2& center, const Inventory& inventory) const
	{
		uint32_t scale = 4;
		float hotbarWidth = atlasTexture.getTileWidth() * inventory.MaxHotbarItems * scale;
		float hotbarHeight = atlasTexture.getTileHeight() * scale;

		glm::vec2 offset = { center.x - hotbarWidth / 2, center.y - hotbarHeight / 2 };

		renderColoredQuad(projectionMatrix, { 0.0, 0.0, 0.0, 0.5 }, offset, glm::vec2(hotbarWidth, hotbarHeight));

		for (int i = 0; i < Inventory::MaxHotbarItems; i++) {
			auto color = i == inventory.getSelectedHotbarSlot() ? glm::vec3(1, 1, 1) : glm::vec3(0.7, 0.7, 0.7);
			renderTile(projectionMatrix, 1023, color, offset, scale);

			auto item = inventory.getHotbarItem(i);
			if (item) {
				auto blockScale = glm::vec3(scale * 6, scale * 6, scale * 6);
				blockScale.y *= -1;
				auto offsetMatrix = glm::translate(glm::mat4(1), glm::vec3(offset.x + atlasTexture.getTileWidth() * scale / 2, offset.y + atlasTexture.getTileHeight() * scale / 2, 0));
				auto rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(45.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1), glm::radians(45.0f), glm::vec3(0, 1, 0));
				auto scaleMatrix = glm::scale(glm::mat4(1), blockScale);
				auto mvp = projectionMatrix * offsetMatrix * rotationMatrix * scaleMatrix;

				defaultShader.use();
				int projectionMatrixLocation = glGetUniformLocation(defaultShader.getID(), "mvp");
				int textureOffsetLocation = glGetUniformLocation(defaultShader.getID(), "textureOffset");
				glUniformMatrix4fv(projectionMatrixLocation, 1, false, glm::value_ptr(mvp));
				glBindTexture(GL_TEXTURE_2D, atlasTexture.getID());
				glBindVertexArray(blockMesh.getVAO());

				auto atlasIndices = BlockAtlasIndicesByType[item->id];

				//top
				auto textureOffset = atlasTexture.getTileOffset(atlasIndices[0]);
				glUniform2f(textureOffsetLocation, textureOffset.x, textureOffset.y);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				//sides
				textureOffset = atlasTexture.getTileOffset(atlasIndices[2]);
				glUniform2f(textureOffsetLocation, textureOffset.x, textureOffset.y);
				glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (void*)(18 * sizeof(uint32_t)));

				glBindVertexArray(0);
				glUseProgram(0);
			}

			offset.x += atlasTexture.getTileWidth() * scale;
		}
	}

	void Renderer::renderTile(const glm::mat4& projectionMatrix, uint32_t id, const glm::vec3& color, const glm::vec2& offset, uint32_t scale) const
	{
		auto scaleMatrix = glm::scale(glm::mat4(1), glm::vec3(scale * atlasTexture.getTileWidth(), scale * atlasTexture.getTileHeight(), scale));
		glm::vec3 offsetVector(offset.x, offset.y, 0);

		uiShader.use();
		int projectionMatrixLocation = glGetUniformLocation(uiShader.getID(), "mvp");
		int textureOffsetLocation = glGetUniformLocation(uiShader.getID(), "textureOffset");
		int colorLocation = glGetUniformLocation(uiShader.getID(), "color");

		glBindTexture(GL_TEXTURE_2D, atlasTexture.getID());
		glBindVertexArray(uiMesh.getVAO());

		auto textureOffset = atlasTexture.getTileOffset(id);
		auto offsetMatrix = glm::translate(glm::mat4(1), offsetVector);
		auto mvp = projectionMatrix * offsetMatrix * scaleMatrix;

		glUniformMatrix4fv(projectionMatrixLocation, 1, false, glm::value_ptr(mvp));
		glUniform2fv(textureOffsetLocation, 1, glm::value_ptr(textureOffset));
		glUniform3f(colorLocation, color.r, color.g, color.b);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(uint32_t)));

		glBindVertexArray(0);
		glUseProgram(0);
	}

	void Renderer::renderColoredQuad(const glm::mat4& projectionMatrix, const glm::vec4& color, const glm::vec2& offset, const glm::vec2& size) const
	{
		glm::vec3 offsetVector(offset.x, offset.y, 0);
		auto scaleMatrix = glm::scale(glm::mat4(1), glm::vec3(size.x, size.y, 1));

		colorShader.use();
		int projectionMatrixLocation = glGetUniformLocation(colorShader.getID(), "mvp");
		int colorLocation = glGetUniformLocation(colorShader.getID(), "color");

		glBindVertexArray(uiMesh.getVAO());

		auto offsetMatrix = glm::translate(glm::mat4(1), offsetVector);
		auto mvp = projectionMatrix * offsetMatrix * scaleMatrix;

		glUniformMatrix4fv(projectionMatrixLocation, 1, false, glm::value_ptr(mvp));
		glUniform4fv(colorLocation, 1, glm::value_ptr(color));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(uint32_t)));

		glBindVertexArray(0);
		glUseProgram(0);
	}

	void Renderer::initUIMesh()
	{
		float characterUVHeight = fontTexture.getTileHeight() / (float)fontTexture.getHeight();
		float characterUVWidth = fontTexture.getTileWidth() / (float)fontTexture.getWidth();

		float tileUVHeight = atlasTexture.getTileHeight() / (float)atlasTexture.getHeight();
		float tileUVWidth = atlasTexture.getTileWidth() / (float)atlasTexture.getWidth();

		std::vector<glm::vec3> positions = {
			{0, 0, 0},
			{0, fontTexture.getTileHeight(), 0},
			{fontTexture.getTileWidth(), fontTexture.getTileHeight(), 0},
			{fontTexture.getTileWidth(), 0, 0},

			{0, 0, 0},
			{0, 1, 0},
			{1, 1, 0},
			{1, 0, 0}
		};

		std::vector<glm::vec2> uvs = {
			{0, 0},
			{0, characterUVHeight},
			{characterUVWidth, characterUVHeight},
			{characterUVWidth, 0},

			{0, 0},
			{0, tileUVHeight},
			{tileUVWidth, tileUVHeight},
			{tileUVWidth, 0}
		};

		std::vector<uint32_t> indices = {
			0, 2, 1, 0, 3, 2, //font
			4, 6, 5, 4, 7, 6 //atlas
		};

		uiMesh.setData(positions, uvs, indices);
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

	void Renderer::initBlockMesh()
	{
		std::vector<glm::vec4> positions;
		std::vector<glm::vec2> uvs;
		std::vector<uint32_t> indices;

		for (int i = 0; i < 6; i++) {
			addBlockFace(positions, uvs, indices, { 0, 0, 0 }, 0, i);
		}

		blockMesh.setData(positions, uvs, indices);
	}

	Renderer::~Renderer()
	{
	}
}