#ifndef YAMC_RENDERER_H
#define YAMC_RENDERER_H

#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Terrain.h"
#include "Inventory.h"
#include "MeshBuilder.h"
#include <glm/glm.hpp>

namespace yamc
{
	class Renderer
	{
	private:
		static constexpr uint32_t FontCharactersPerRow = 16;
		static constexpr uint32_t FontRowCount = 8;
		static constexpr uint32_t FontCharacterOffset = 1;

		static constexpr uint32_t AtlasTilesPerRow = 32;
		static constexpr uint32_t AtlasRowCount = 32;

		static constexpr char* FontBitmapPath = "data/images/font.png";
		static constexpr char* AtlasPath = "data/images/atlas.png";

		static constexpr char* UIVertexShaderPath = "data/shaders/ui.vert";
		static constexpr char* UIFragmentShaderPath = "data/shaders/ui.frag";
		
	    static constexpr char* ColorVertexShaderPath = "data/shaders/color.vert";
		static constexpr char* ColorFragmentShaderPath = "data/shaders/color.frag";

		static constexpr char* DefaultVertexShaderPath = "data/shaders/default.vert";
		static constexpr char* DefaultFragmentShaderPath = "data/shaders/default.frag";

	public:
		Renderer();
		Renderer(Renderer&&) = default;
		Renderer(const Renderer&) = delete;
		void renderText(const glm::mat4& projectionMatrix, const std::string& text, const glm::vec3& color, const glm::vec2& offset, uint32_t scale) const;
		void renderCubeOutline(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& center) const;
		void renderCross(const glm::mat4& projectionMatrix, const glm::vec2& center) const;
		void renderTerrain(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const Terrain& terrain, int visibleChunksRadius, const glm::vec3 cameraPosition) const;
		void renderInventoryHotbar(const glm::mat4& projectionMatrix, const glm::vec2& center, const Inventory& inventory) const;
		void renderTile(const glm::mat4& projectionMatrix, uint32_t id, const glm::vec3& color, const glm::vec2& offset, uint32_t scale) const;
		void renderColoredQuad(const glm::mat4& projectionMatrix, const glm::vec4& color, const glm::vec2& offset, const glm::vec2& size) const;
		void renderBlockIcon(const glm::mat4& projectionMatrix, uint32_t id, const glm::vec2& center, uint32_t scale) const;
		const MeshBuilder& getMeshBuilder() const;
		~Renderer();

	private:
		Shader uiShader;
		Shader colorShader;
		Shader defaultShader;

		AtlasTexture fontTexture;
		AtlasTexture atlasTexture;
		
		Mesh quadMesh;
		Mesh outlineMesh;
		Mesh blockMesh;

		MeshBuilder meshBuilder;

		void initQuadMesh();
		void initOutlineMesh();
		void initBlockMesh();
	};
}

#endif