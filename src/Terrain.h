#ifndef YAMC_TERRAIN_H
#define YAMC_TERRAIN_H

#include <unordered_map>
#include <stdint.h>
#include "Chunk.h"

namespace yamc
{
	class Terrain
	{
	public:
		Terrain();
		uint32_t getBlock(int x, int y, int z) const;
		void setBlock(int x, int y, int z, uint32_t id);
		const std::unordered_map<uint64_t, Chunk*>& getChunks() const;
		static glm::ivec2 getChunkOffset(uint64_t key);
		~Terrain();

	private:
		std::unordered_map<uint64_t, Chunk*> chunks;
		bool findBlock(Chunk** chunk, uint32_t& localX, uint32_t& localY, int x, int y, int z) const;
	};
}

#endif