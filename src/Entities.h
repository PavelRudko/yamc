#ifndef YAMC_ENTITIES_H
#define YAMC_ENTITIES_H

#include <glm/glm.hpp>
#include <vector>

namespace yamc
{
	class Terrain;

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
	void pushEntityToTheTop(const Terrain& terrain, Entity* entity);
	bool resolveEntityTerrainCollisions(Entity* entity, const std::vector<glm::vec3>& potentialBlocks);
	void updateEntityPosition(const Terrain& terrain, Entity* entity, float dt);
}

#endif