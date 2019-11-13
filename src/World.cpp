#include "World.h"
#include <algorithm>
#include <filesystem>
#include <string>

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

	void resolveEntityTerrainCollisions(Entity* entity, const std::vector<glm::vec3>& potentialBlocks)
	{
		static float bias = 0.00001f;
		entity->isGrounded = false;

		for (auto block : potentialBlocks) {
			AABB blockBoundingBox;
			blockBoundingBox.center = block + glm::vec3(0.5f, 0.5f, 0.5f);
			blockBoundingBox.halfSize = glm::vec3(0.5f, 0.5f, 0.5f);

			glm::vec3 normal;
			float penetration = 0;
			if (getContact(entity->boundingBox, blockBoundingBox, normal, penetration)) {
				entity->boundingBox.center += normal * (penetration + bias);
				float velocityProjection = glm::dot(entity->velocity, normal);
				entity->velocity -= normal * velocityProjection;
				if (normal.y > 0) {
					entity->isGrounded = true;
				}
			}
		}
	}

	float getDistanceToBlockCenter(const Entity* entity, const glm::vec3& block)
	{
		auto blockCenter = block + glm::vec3(0.5f, 0.5f, 0.5f);
		return glm::length(blockCenter - entity->boundingBox.center);
	}

	World::World(const std::string& name, int seed) :
		seed(seed),
		directoryName("worlds/" + name)
	{
		ensureWorldDirectoryExists();

		player.boundingBox.center = glm::vec3(0, 0, 0);
		player.boundingBox.halfSize = glm::vec3(0.4f, 0.9f, 0.4f);
		player.velocity = glm::vec3(0, 0, 0);
		player.isGrounded = false;

		trackedEntities.push_back(&player);

		loadSurroundingChunks(MinSurroundingChunksRadius);

		pushEntityToTheTop(&player);
	}

	void World::update(float dt)
	{
		loadSurroundingChunks(MinSurroundingChunksRadius);
		if (terrain.getChunks().size() > MaxChunksInMemory) {
			unloadDistantChunks(PurgeRemainingChunksRadius);
		}

		for (auto& entity : trackedEntities) {
			updateEntityPosition(entity, dt);
		}
	}

	void World::saveWorld()
	{
		auto chunks = terrain.getChunks();
		for (uint64_t key : terrain.getDirtyChunkKeys()) {
			writeChunkToFile(getChunkPath(key), chunks[key]);
		}
	}

	void World::updateEntityPosition(Entity* entity, float dt)
	{
		entity->boundingBox.center += entity->velocity * dt;

		auto min = entity->boundingBox.center - entity->boundingBox.halfSize;
		auto max = entity->boundingBox.center + entity->boundingBox.halfSize;

		glm::ivec3 imin = glm::floor(min);
		glm::ivec3 imax = glm::ceil(max);

		std::vector<glm::vec3> blockCollisionCandidates;
		for (int x = imin.x; x <= imax.x; x++) {
			for (int y = imin.y; y <= imax.y; y++) {
				for (int z = imin.z; z <= imax.z; z++) {
					if (getTerrain().getBlock(x, y, z) > 0) {
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
		resolveEntityTerrainCollisions(entity, blockCollisionCandidates);
	}

	glm::ivec2 getMinMaxChunkOffsets(float coordinate, uint32_t radius, uint32_t chunkSize)
	{
		float extent = radius * chunkSize;
		int minWorldIndex = floorf(coordinate - extent);
		int maxWorldIndex = floorf(coordinate + extent);
		return glm::ivec2(getChunkIndex(minWorldIndex, chunkSize), getChunkIndex(maxWorldIndex, chunkSize));
	}

	void World::loadSurroundingChunks(uint32_t radius)
	{
		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();

		auto playerCenter = player.boundingBox.center;
		auto boundariesX = getMinMaxChunkOffsets(playerCenter.x, radius, Chunk::MaxWidth);
		auto boundariesZ = getMinMaxChunkOffsets(playerCenter.z, radius, Chunk::MaxLength);

		for (int x = boundariesX[0]; x <= boundariesX[1]; x++) {
			for (int z = boundariesZ[0]; z <= boundariesZ[1]; z++) {
				auto key = getChunkKey(x, z);
				if (chunks.find(key) == chunks.end()) {
					loadChunk(x, z);
				}
			}
		}
	}

	void World::loadChunk(int offsetX, int offsetZ)
	{
		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();
		auto chunk = new Chunk();
		uint64_t chunkKey = getChunkKey(offsetX, offsetZ);

		FILE* file = fopen(getChunkPath(chunkKey).c_str(), "rb");
		if (file) {
			fread(chunk->getData(), sizeof(uint32_t), Chunk::MaxHeight * Chunk::MaxLength * Chunk::MaxWidth, file);
			fclose(file);
		}
		else {
			fillChunk(chunk, offsetX, offsetZ, seed);
		}
		
		chunk->update();
		chunks[chunkKey] = chunk;
	}

	void World::unloadDistantChunks(uint32_t remainingRadius)
	{
		auto playerCenter = player.boundingBox.center;
		auto boundariesX = getMinMaxChunkOffsets(playerCenter.x, remainingRadius, Chunk::MaxWidth);
		auto boundariesZ = getMinMaxChunkOffsets(playerCenter.z, remainingRadius, Chunk::MaxLength);

		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();
		std::set<uint64_t>& dirtyChunkKeys = terrain.getDirtyChunkKeys();

		for (auto it = chunks.begin(); it != chunks.end();) {
			auto offset = getChunkOffset(it->first);
			if (offset[0] < boundariesX[0] || offset[0] > boundariesX[1] ||
				offset[1] < boundariesZ[0] || offset[1] > boundariesZ[1]) {
				auto chunk = it->second;
				if (dirtyChunkKeys.count(it->first) > 0) {
					writeChunkToFile(getChunkPath(it->first), chunk);
				}

				it = chunks.erase(it);
				delete chunk;
			}
			else {
				it++;
			}
		}
	}

	void World::pushEntityToTheTop(Entity* entity) const
	{
		auto minBoundary = entity->boundingBox.center - entity->boundingBox.halfSize;
		auto maxBoundary = entity->boundingBox.center + entity->boundingBox.halfSize;
		auto minBlockCoordinate = glm::ivec3(glm::floor(minBoundary));
		auto maxBlockCoordinate = glm::ivec3(glm::floor(maxBoundary));

		int maxY = 0;
		for (int x = minBlockCoordinate.x; x <= maxBlockCoordinate.x; x++) {
			for (int z = minBlockCoordinate.z; z <= maxBlockCoordinate.z; z++) {
				for (int y = Chunk::MaxHeight - 1; y > 0; y--) {
					if (terrain.getBlock(x, y, z) != 0 && (y + 1) > maxY) {
						maxY = y + 1;
					}
				}
			}
		}

		entity->boundingBox.center.y = maxY + entity->boundingBox.halfSize.y + 0.0001f;
	}

	void World::writeChunkToFile(const std::string& path, const Chunk* chunk) const
	{
		FILE* file = fopen(path.c_str(), "wb");
		fwrite(chunk->getData(), sizeof(uint32_t), Chunk::MaxHeight * Chunk::MaxLength * Chunk::MaxWidth, file);
		fclose(file);
	}

	std::string World::getChunkPath(uint64_t key) const
	{
		return directoryName + "/" + std::to_string(key) + ".cnk";
	}

	void World::ensureWorldDirectoryExists() const
	{
		std::string worldsRootDirectory("worlds");
		if (!std::filesystem::is_directory(worldsRootDirectory) || !std::filesystem::exists(worldsRootDirectory)) {
			std::filesystem::create_directory(worldsRootDirectory);
		}

		if (!std::filesystem::is_directory(directoryName) || !std::filesystem::exists(directoryName)) {
			std::filesystem::create_directory(directoryName);
		}
	}

	const Terrain& World::getTerrain() const
	{
		return terrain;
	}

	Terrain& World::getTerrain()
	{
		return terrain;
	}

	const Entity& World::getPlayer() const
	{
		return player;
	}

	Entity& World::getPlayer()
	{
		return player;
	}

	World::~World()
	{
	}
}