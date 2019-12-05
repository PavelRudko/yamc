#ifndef YAMC_WORLD_DATA_MANAGER_H
#define YAMC_WORLD_DATA_MANAGER_H

#include <string>
#include "Terrain.h"

namespace yamc
{
	class WorldDataManager
	{
	public:
		WorldDataManager(int seed, const std::string& worldName);
		Chunk* loadChunk(uint64_t key);
		void saveChunk(uint64_t key, const Chunk* chunk) const;

	private:
		int seed;
		std::string directoryPath;

		std::string getChunkPath(uint64_t key) const;
		void ensureWorldDirectoryExists() const;
	};
}

#endif