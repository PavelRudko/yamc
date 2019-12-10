#include "Entities.h"
#include "Terrain.h"
#include <algorithm>

namespace yamc
{
	bool hasIntersection(const AABB& a, const AABB& b)
	{
		for (int i = 0; i < 3; i++) {
			float aMin = a.center[i] - a.halfSize[i];
			float aMax = a.center[i] + a.halfSize[i];
			float bMin = b.center[i] - b.halfSize[i];
			float bMax = b.center[i] + b.halfSize[i];

			if (bMax < aMin) {
				return false;
			}

			if (bMin > aMax) {
				return false;
			}
		}
		return true;
	}

	bool getContact(const AABB& a, const AABB& b, glm::vec3& normal, float& penetration)
	{
		penetration = std::numeric_limits<float>().infinity();
		auto diff = a.center - b.center;

		for (int i = 0; i < 3; i++) {
			float axisDiff = diff[i];
			float ra = a.halfSize[i];
			float rb = b.halfSize[i];

			if (abs(axisDiff) >= ra + rb) {
				return false;
			}

			float p = abs(axisDiff) - ra;
			p = rb - p;
			if (p < penetration) {
				normal = glm::vec3(0, 0, 0);
				normal[i] = axisDiff > 0 ? 1.0f : -1.0f;
				penetration = p;
			}
		}
		return true;
	}

	bool resolveEntityTerrainCollisions(Entity* entity, const std::vector<glm::vec3>& potentialBlocks)
	{
		static float bias = 0.00001f;
		bool hasCollision = false;

		for (auto block : potentialBlocks) {
			AABB blockBoundingBox;
			blockBoundingBox.center = block + glm::vec3(0.5f, 0.5f, 0.5f);
			blockBoundingBox.halfSize = glm::vec3(0.5f, 0.5f, 0.5f);

			glm::vec3 normal;
			float penetration = 0;
			if (getContact(entity->boundingBox, blockBoundingBox, normal, penetration)) {
				hasCollision = true;
				entity->boundingBox.center += normal * (penetration + bias);
				float velocityProjection = glm::dot(entity->velocity, normal);
				entity->velocity -= normal * velocityProjection;
				if (normal.y > 0) {
					entity->isGrounded = true;
				}
			}
		}

		return hasCollision;
	}

	float getDistanceToBlockCenter(const Entity* entity, const glm::vec3& block)
	{
		auto blockCenter = block + glm::vec3(0.5f, 0.5f, 0.5f);
		return glm::length(blockCenter - entity->boundingBox.center);
	}

	void updateEntityPosition(const Terrain& terrain, Entity* entity, float dt)
	{
		static float maxStepLength = 0.5f;

		auto originalPosition = entity->boundingBox.center;
		auto direction = glm::normalize(entity->velocity);
		auto delta = entity->velocity * dt;
		auto deltaLength = glm::length(delta);
		entity->isGrounded = false;
		for (float distance = maxStepLength; distance < deltaLength + maxStepLength; distance += maxStepLength) {
			float t = std::min(distance, deltaLength);
			entity->boundingBox.center = originalPosition + direction * t;

			auto min = entity->boundingBox.center - entity->boundingBox.halfSize;
			auto max = entity->boundingBox.center + entity->boundingBox.halfSize;

			glm::ivec3 imin = glm::floor(min);
			glm::ivec3 imax = glm::ceil(max);

			std::vector<glm::vec3> blockCollisionCandidates;
			for (int x = imin.x; x <= imax.x; x++) {
				for (int y = imin.y; y <= imax.y; y++) {
					for (int z = imin.z; z <= imax.z; z++) {
						if (terrain.getBlock(x, y, z) > 0) {
							blockCollisionCandidates.push_back(glm::vec3(x, y, z));
						}
					}
				}
			}

			std::sort(blockCollisionCandidates.begin(), blockCollisionCandidates.end(),
				[&](const glm::vec3& a, const glm::vec3& b) -> bool
			{
				return getDistanceToBlockCenter(entity, a) < getDistanceToBlockCenter(entity, b);
			});

			if (resolveEntityTerrainCollisions(entity, blockCollisionCandidates)) {
				break;
			}
		}
	}

	bool terrainContainsBlocksAtY(const Terrain& terrain, const glm::ivec3& minBlockCoordinate, const glm::ivec3& maxBlockCoordinate, int y)
	{
		for (int x = minBlockCoordinate.x; x <= maxBlockCoordinate.x; x++) {
			for (int z = minBlockCoordinate.z; z <= maxBlockCoordinate.z; z++) {
				if (terrain.getBlock(x, y, z) != 0) {
					return true;
				}
			}
		}

		return false;
	}

	void pushEntityToTheTop(const Terrain& terrain, Entity* entity)
	{
		auto minBoundary = entity->boundingBox.center - entity->boundingBox.halfSize;
		auto maxBoundary = entity->boundingBox.center + entity->boundingBox.halfSize;
		auto minBlockCoordinate = glm::ivec3(glm::floor(minBoundary));
		auto maxBlockCoordinate = glm::ivec3(glm::floor(maxBoundary));

		for (int y = Chunk::MaxHeight - 1; y > 0; y--) {
			if (terrainContainsBlocksAtY(terrain, minBlockCoordinate, maxBlockCoordinate, y)) {
				entity->boundingBox.center.y = y + 1 + entity->boundingBox.halfSize.y + 0.1f;
				return;
			}
		}
	}
}