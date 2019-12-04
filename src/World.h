#ifndef YAMC_WORLD_H
#define YAMC_WORLD_H

#include "Terrain.h"

namespace yamc
{
	class World
	{
	public:
		World(const std::string& name, int seed, uint32_t visibleChunksRadius);
		World(const World&) = delete;
		World(World&& other) = default;

		void update(float dt);
		void saveWorld();
		void loadSurroundingChunks(const glm::vec3& position, uint32_t radius);

		const Terrain& getTerrain() const;
		Terrain& getTerrain();

		~World();

	private:
		Terrain terrain;
		int seed;
		std::string directoryName;
		uint32_t minSurroundingChunksRadius;
		uint32_t purgeRemainingChunksRadius;
		uint32_t maxChunksInMemory;

		void loadChunk(int offsetX, int offsetZ);
		void unloadDistantChunks(const glm::vec3& position, uint32_t remainingRadius);
		
		void writeChunkToFile(const std::string& path, const Chunk* chunk) const;
		std::string getChunkPath(uint64_t key) const;
		void ensureWorldDirectoryExists() const;
	};
}

#endif