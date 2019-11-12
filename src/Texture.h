#ifndef YAMC_TEXTURE_H
#define YAMC_TEXTURE_H

#include <stdint.h>
#include <string>

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
}

#endif