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
	private:
		static constexpr uint32_t MinSurroundingChunksRadius = 1;
		static constexpr uint32_t PurgeRemainingChunksRadius = 3;
		static constexpr uint32_t MaxChunksInMemory = 25;

	public:
		World(const std::string& name, int seed);
		World(const World&) = delete;
		World(World&& other) = default;

		void update(float dt);

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

		void updateEntityPosition(Entity* entity, float dt);
		void loadSurroundingChunks(uint32_t radius);
		void loadChunk(int offsetX, int offsetZ);
		void unloadDistantChunks(uint32_t remainingRadius);
		void pushEntityToTheTop(Entity* entity) const;
	};
}

#endif