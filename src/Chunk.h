#ifndef YAMC_CHUNK_H
#define YAMC_CHUNK_H

#include <stdint.h>
#include <glm/glm.hpp>
#include "Mesh.h"

namespace yamc
{
	class Chunk
	{
	public:
		static constexpr uint32_t MaxWidth = 16;
		static constexpr uint32_t MaxLength = 16;
		static constexpr uint32_t MaxHeight = 128;

		Chunk();
		Chunk(const Chunk&) = delete;
		Chunk(Chunk&&) = delete;

		uint32_t getBlock(uint32_t x, uint32_t y, uint32_t z) const;
		void setBlock(uint32_t x, uint32_t y, uint32_t z, uint32_t value);
		const Mesh& getMesh() const;
		Mesh& getMesh();
		const void* getData() const;
		void* getData();

		~Chunk();

	private:
		uint32_t blocks[MaxWidth][MaxHeight][MaxLength];
		Mesh mesh;

		bool isFaceVisible(int x, int y, int z, const glm::ivec3& normal) const;
	};
}

#endif
