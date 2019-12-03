#ifndef YAMC_WORLD_H
#define YAMC_WORLD_H

#include "Terrain.h"

namespace yamc
{
	struct AABB
	{
		glm::vec3 center;
		glm::vec3 halfSize;
	};

	struct Entity
	{
		AABB boundingBox;
		glm::vec3 velocity;
		bool isGrounded;
	};

	struct BlockSelection
	{
		bool isSelected;
		glm::ivec3 coordinate;
		glm::ivec3 normal;
	};

	bool hasIntersection(const AABB& a, const AABB& b);

	class World
	{
	public:
		World(const std::string& name, int seed, uint32_t visibleChunksRadius);
		World(const World&) = delete;
		World(World&& other) = default;

		void update(float dt);
		void saveWorld();

		const Terrain& getTerrain() const;
		Terrain& getTerrain();

		const Entity& getPlayer() const;
		Entity& getPlayer();

		~World();

	private:
		Entity player;
		Terrain terrain;
		std::vector<Entity*> trackedEntities;
		int seed;
		std::string directoryName;
		uint32_t minSurroundingChunksRadius;
		uint32_t purgeRemainingChunksRadius;
		uint32_t maxChunksInMemory;

		void updateEntityPosition(Entity* entity, float dt);
		void loadSurroundingChunks(uint32_t radius);
		void loadChunk(int offsetX, int offsetZ);
		void unloadDistantChunks(uint32_t remainingRadius);
		void pushEntityToTheTop(Entity* entity) const;
		void writeChunkToFile(const std::string& path, const Chunk* chunk) const;
		std::string getChunkPath(uint64_t key) const;
		void ensureWorldDirectoryExists() const;
	};
}

#endif