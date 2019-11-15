#ifndef YAMC_TERRAIN_H
#define YAMC_TERRAIN_H

#include <unordered_map>
#include <set>
#include <stdint.h>
#include "Chunk.h"

namespace yamc
{
	int getChunkIndex(int worldIndex, int chunkSize);
	uint64_t getChunkKey(int high, int low);
	glm::ivec2 getChunkOffset(uint64_t key);
	void fillChunk(Chunk* chunk, int offsetX, int offsetZ, int seed);

	class Terrain
	{
	public:
		Terrain();
		uint32_t getBlock(int x, int y, int z) const;
		void setBlock(int x, int y, int z, uint32_t id);
		const std::unordered_map<uint64_t, Chunk*>& getChunks() const;
		std::unordered_map<uint64_t, Chunk*>& getChunks();
		const std::set<uint64_t>& getChunkKeysToSave() const;
		std::set<uint64_t>& getChunkKeysToSave();
		const std::set<uint64_t>& getChunkKeysToRebuild() const;
		std::set<uint64_t>& getChunkKeysToRebuild();
		~Terrain();

	private:
		std::unordered_map<uint64_t, Chunk*> chunks;
		std::set<uint64_t> chunkKeysToSave;
		std::set<uint64_t> chunkKeysToRebuild;
		bool findBlock(Chunk** chunk, uint32_t& localX, uint32_t& localY, uint64_t& chunkKey, int x, int y, int z) const;
	};
}

#endif