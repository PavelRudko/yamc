#ifndef YAMC_TEXTURE_H
#define YAMC_TEXTURE_H

#include <stdint.h>
#include <string>
#include <glm/glm.hpp>

namespace yamc
{
	class Texture
	{
	public:
		Texture(const std::string& path);
		Texture(const char* path);
		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept;

		uint32_t getID() const;
		uint32_t getWidth() const;
		uint32_t getHeight() const;

		~Texture();

	private:
		uint32_t id;
		uint32_t width;
		uint32_t height;
	};

	class AtlasTexture : public Texture
	{
	public:
		AtlasTexture(const std::string& path, uint32_t tilesPerRow, uint32_t rowCount);
		AtlasTexture(AtlasTexture&&) noexcept;

		uint32_t getTilesPerRow() const;
		uint32_t getRowCount() const;
		uint32_t getTileWidth() const;
		uint32_t getTileHeight() const;
		glm::vec2 getTileOffset(uint32_t index) const;
		glm::vec2 getUVScale() const;

	private:
		uint32_t tilesPerRow;
		uint32_t rowCount;
		uint32_t tileWidth;
		uint32_t tileHeight;
	};
}

#endif