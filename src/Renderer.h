#ifndef YAMC_RENDERER_H
#define YAMC_RENDERER_H

#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Terrain.h"
#include <glm/glm.hpp>

namespace yamc
{
	class Renderer
	{
	private:
		static constexpr uint32_t FontCharactersPerRow = 16;
		static constexpr uint32_t FontRowCount = 8;
		static constexpr uint32_t FontCharacterOffset = 1;
		static constexpr char* FontBitmapPath = "data/images/font.png";
		static constexpr char* FontVertexShaderPath = "data/shaders/font.vert";
		static constexpr char* FontFragmentShaderPath = "data/shaders/font.frag";
		
	    static constexpr char* ColorVertexShaderPath = "data/shaders/color.vert";
		static constexpr char* ColorFragmentShaderPath = "data/shaders/color.frag";

		static constexpr char* AtlasPath = "data/images/atlas.png";
		static constexpr char* DefaultVertexShaderPath = "data/shaders/default.vert";
		static constexpr char* DefaultFragmentShaderPath = "data/shaders/default.frag";

	public:
		Renderer();
		Renderer(Renderer&&) = default;
		Renderer(const Renderer&) = delete;
		void renderText(const glm::mat4& projectionMatrix, const std::string& text, const glm::vec3& color, const glm::vec2& offset, uint32_t scale) const;
		void renderCubeOutline(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& center) const;
		void renderCross(const glm::mat4& projectionMatrix, const glm::vec2& center) const;
		void renderTerrain(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const Terrain& terrain) const;
		~Renderer();

	private:
		Texture fontTexture;
		Shader fontShader;
		Mesh fontCharacterMesh;
		uint32_t fontCharacterWidth;
		uint32_t fontCharacterHeight;

		Shader colorShader;
		Mesh outlineMesh;

		Shader defaultShader;
		Texture atlasTexture;

		void initFont();
		void initOutlineMesh();
		glm::vec2 getFontTextureOffset(uint8_t character) const;
	};
}

#endif