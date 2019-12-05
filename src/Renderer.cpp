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
		defaultShader(DefaultVertexShaderPath, DefaultFragmentShaderPath),
		meshBuilder(atlasTexture)
	{
		initQuadMesh();
		initOutlineMesh();
		initBlockMesh();
	}

	void Renderer::renderText(const glm::mat4& projectionMatrix, const std::string& text, const glm::vec3& color, const glm::vec2& offset, uint32_t scale) const
	{
		auto scaleMatrix = glm::scale(glm::mat4(1), glm::vec3(scale * fontTexture.getTileWidth(), scale * fontTexture.getTileHeight(), 1));
		glm::vec3 characterOffsetVector(offset.x, offset.y, 0);
	
		uiShader.use();

		glBindTexture(GL_TEXTURE_2D, fontTexture.getID());
		glBindVertexArray(quadMesh.getVAO());

		for (int i = 0; i < text.length(); i++) {
			characterOffsetVector.x += (FontCharacterOffset + fontTexture.getTileWidth()) * scale;
			auto offsetMatrix = glm::translate(glm::mat4(1), characterOffsetVector);

			uiShader.setMVP(projectionMatrix * offsetMatrix * scaleMatrix);
			uiShader.setTextureOffset(fontTexture.getTileOffset(text[i]));
			uiShader.setColor(color);
			uiShader.setTextureScale(fontTexture.getUVScale());
			glDrawElements(GL_TRIANGLES, quadMesh.getIndicesCount(), GL_UNSIGNED_INT, 0);
		}
		
		glBindVertexArray(0);
		glUseProgram(0);
	}

	void Renderer::renderCubeOutline(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& center) const
	{
		glUseProgram(colorShader.getID());

		auto modelMatrix = glm::translate(glm::identity<glm::mat4>(), center);
		glm::vec4 outlineColor(0.6f, 0.6f, 0.6f, 1.0f);
		colorShader.setMVP(projectionMatrix * viewMatrix * modelMatrix);
		colorShader.setColor(outlineColor);
		glLineWidth(3);

		glBindVertexArray(outlineMesh.getVAO());
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(0);
	}

	void Renderer::renderCross(const glm::mat4& projectionMatrix, const glm::vec2& center) const
	{
		glUseProgram(colorShader.getID());

		auto modelMatrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(center.x, center.y, 0));
		auto mvp = projectionMatrix * modelMatrix * glm::scale(glm::identity<glm::mat4>(), glm::vec3(26, 26, 26));
		glm::vec4 crossColor(0.6f, 0.6f, 0.6f, 0.6f);
		colorShader.setMVP(projectionMatrix * modelMatrix * glm::scale(glm::identity<glm::mat4>(), glm::vec3(26, 26, 26)));
		colorShader.setColor(crossColor);
		glLineWidth(4);

		glBindVertexArray(outlineMesh.getVAO());
		glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, (void*)(24 * sizeof(uint32_t)));
		glBindVertexArray(0);

		glUseProgram(0);
	}

	void Renderer::renderTerrain(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::unordered_map<uint64_t, Mesh*>& chunkMeshes, int visibleChunksRadius, const glm::vec3 cameraPosition) const
	{
		defaultShader.use();
		glBindTexture(GL_TEXTURE_2D, atlasTexture.getID());

		defaultShader.setTextureOffset(glm::vec2(0, 0));

		int cameraChunkX = getChunkIndex(cameraPosition.x, Chunk::MaxWidth);
		int cameraChunkZ = getChunkIndex(cameraPosition.z, Chunk::MaxLength);

		for (int x = cameraChunkX - visibleChunksRadius; x <= cameraChunkX + visibleChunksRadius; x++) {
			for (int z = cameraChunkZ - visibleChunksRadius; z <= cameraChunkZ + visibleChunksRadius; z++) {
				uint64_t key = getChunkKey(x, z);
				auto pair = chunkMeshes.find(key);
				if (pair != chunkMeshes.end()) {
					glm::vec3 offset(x * (float)Chunk::MaxWidth, 0, z * (float)Chunk::MaxLength);

					auto mesh = pair->second;
					if (mesh->getIndicesCount() > 0) {
						auto modelMatrix = glm::translate(glm::identity<glm::mat4>(), offset);
						defaultShader.setMVP(projectionMatrix * viewMatrix * modelMatrix);

						glBindVertexArray(mesh->getVAO());
						glDrawElements(GL_TRIANGLES, mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
						glBindVertexArray(0);
					}
				}
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
				glm::vec2 blockIconCenter(offset.x + atlasTexture.getTileWidth() * scale / 2, offset.y + atlasTexture.getTileHeight() * scale / 2);
				renderBlockIcon(projectionMatrix, item->id, blockIconCenter, scale);
			}

			offset.x += atlasTexture.getTileWidth() * scale;
		}
	}

	void Renderer::renderTile(const glm::mat4& projectionMatrix, uint32_t id, const glm::vec3& color, const glm::vec2& offset, uint32_t scale) const
	{
		auto scaleMatrix = glm::scale(glm::mat4(1), glm::vec3(scale * atlasTexture.getTileWidth(), scale * atlasTexture.getTileHeight(), scale));
		glm::vec3 offsetVector(offset.x, offset.y, 0);

		uiShader.use();

		glBindTexture(GL_TEXTURE_2D, atlasTexture.getID());
		glBindVertexArray(quadMesh.getVAO());

		auto textureOffset = atlasTexture.getTileOffset(id);
		auto offsetMatrix = glm::translate(glm::mat4(1), offsetVector);

		uiShader.setMVP(projectionMatrix * offsetMatrix * scaleMatrix);
		uiShader.setTextureOffset(textureOffset);
		uiShader.setColor(color);
		uiShader.setTextureScale(atlasTexture.getUVScale());
		glDrawElements(GL_TRIANGLES, quadMesh.getIndicesCount(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	void Renderer::renderColoredQuad(const glm::mat4& projectionMatrix, const glm::vec4& color, const glm::vec2& offset, const glm::vec2& size) const
	{
		glm::vec3 offsetVector(offset.x, offset.y, 0);
		auto scaleMatrix = glm::scale(glm::mat4(1), glm::vec3(size.x, size.y, 1));

		colorShader.use();

		glBindVertexArray(quadMesh.getVAO());

		auto offsetMatrix = glm::translate(glm::mat4(1), offsetVector);
		auto mvp = projectionMatrix * offsetMatrix * scaleMatrix;

		colorShader.setMVP(mvp);
		colorShader.setColor(color);
		glDrawElements(GL_TRIANGLES, quadMesh.getIndicesCount(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	void Renderer::renderBlockIcon(const glm::mat4& projectionMatrix, uint32_t id, const glm::vec2& center, uint32_t scale) const
	{
		glm::vec3 scaleVector (scale * 6, scale * 6, scale * 6);
		scaleVector.y *= -1;
		auto offsetMatrix = glm::translate(glm::mat4(1), glm::vec3(center.x, center.y, 0));
		auto rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(45.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1), glm::radians(45.0f), glm::vec3(0, 1, 0));
		auto scaleMatrix = glm::scale(glm::mat4(1), scaleVector);
		auto atlasIndices = MeshBuilder::BlockAtlasIndicesByType[id];

		defaultShader.use();

		defaultShader.setMVP(projectionMatrix * offsetMatrix * rotationMatrix * scaleMatrix);
		glBindTexture(GL_TEXTURE_2D, atlasTexture.getID());
		glBindVertexArray(blockMesh.getVAO());

		//top
		defaultShader.setTextureOffset(atlasTexture.getTileOffset(atlasIndices[0]));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//sides
		defaultShader.setTextureOffset(atlasTexture.getTileOffset(atlasIndices[2]));
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (void*)(18 * sizeof(uint32_t)));

		glBindVertexArray(0);
		glUseProgram(0);
	}

	const MeshBuilder& Renderer::getMeshBuilder() const
	{
		return meshBuilder;
	}

	void Renderer::initQuadMesh()
	{
		std::vector<glm::vec3> positions = {
			{0, 0, 0},
			{0, 1, 0},
			{1, 1, 0},
			{1, 0, 0}
		};

		std::vector<glm::vec2> uvs = {
			{0, 0},
			{0, 1},
			{1, 1},
			{1, 0},
		};

		std::vector<uint32_t> indices = {
			0, 2, 1, 0, 3, 2
		};

		quadMesh.setData(positions, uvs, indices);
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

		meshBuilder.addCubeBlock(positions, uvs, indices, { 0, 0, 0 }, {0, 0, 0}, 0);

		blockMesh.setData(positions, uvs, indices);
	}

	Renderer::~Renderer()
	{
	}
}