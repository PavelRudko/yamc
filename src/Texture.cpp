#include "Texture.h"
#include <gl3w.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stdexcept>

namespace yamc
{
	Texture::Texture(const std::string& path) :
		Texture(path.c_str())
	{
		
	}

	Texture::Texture(const char* path) :
		id(0)
	{
		int w, h, c;
		unsigned char* data = stbi_load(path, &w, &h, &c, 0);
		if (!data) {
			throw std::runtime_error("Cannot open texure image " + std::string(path));
		}

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);

		glBindTexture(GL_TEXTURE_2D, 0);

		width = w;
		height = h;
	}

	Texture::Texture(Texture&& other) noexcept
	{
		width = other.width;
		height = other.height;
		id = other.id;
		other.id = 0;
	}

	uint32_t Texture::getID() const
	{
		return id;
	}

	uint32_t Texture::getWidth() const
	{
		return width;
	}

	uint32_t Texture::getHeight() const
	{
		return height;
	}

	Texture::~Texture()
	{
		if (id > 0) {
			glDeleteTextures(1, &id);
		}
	}

	AtlasTexture::AtlasTexture(const std::string& path, uint32_t tilesPerRow, uint32_t rowCount) :
		Texture(path),
		tilesPerRow(tilesPerRow),
		rowCount(rowCount)
	{
		tileWidth = getWidth() / tilesPerRow;
		tileHeight = getHeight() / rowCount;
	}

	AtlasTexture::AtlasTexture(AtlasTexture&& other) noexcept : 
		Texture(std::move(other)),
		tilesPerRow(other.tilesPerRow),
		rowCount(other.rowCount),
		tileWidth(other.tileWidth),
		tileHeight(other.tileHeight)
	{
		
	}

	uint32_t AtlasTexture::getTilesPerRow() const
	{
		return tilesPerRow;
	}

	uint32_t AtlasTexture::getRowCount() const
	{
		return rowCount;
	}

	uint32_t AtlasTexture::getTileWidth() const
	{
		return tileWidth;
	}

	uint32_t AtlasTexture::getTileHeight() const
	{
		return tileHeight;
	}

	glm::vec2 AtlasTexture::getTileOffset(uint32_t index) const
	{
		uint32_t row = ((uint32_t)index) / tilesPerRow;
		uint32_t column = ((uint32_t)index) % tilesPerRow;
		return glm::vec2(column / (float)tilesPerRow, row / (float)rowCount);
	}

}
