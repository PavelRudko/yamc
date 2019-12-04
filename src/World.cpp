#include "World.h"
#include <algorithm>
#include <filesystem>
#include <string>
#include "Entities.h"

namespace yamc
{
	World::World(const std::string& name, int seed, uint32_t visibleChunksRadius) :
		seed(seed),
		directoryName("worlds/" + name)
	{
		ensureWorldDirectoryExists();

		minSurroundingChunksRadius = visibleChunksRadius + 1;
		purgeRemainingChunksRadius = visibleChunksRadius + 2;
		maxChunksInMemory = pow((purgeRemainingChunksRadius * 2 + 1), 2) * 3;
	}

	void World::update(float dt)
	{
	}

	void World::saveWorld()
	{
		auto chunks = terrain.getChunks();
		for (uint64_t key : terrain.getChunkKeysToSave()) {
			writeChunkToFile(getChunkPath(key), chunks[key]);
		}
	}

	glm::ivec2 getMinMaxChunkOffsets(float coordinate, uint32_t radius, uint32_t chunkSize)
	{
		float extent = radius * chunkSize;
		int minWorldIndex = floorf(coordinate - extent);
		int maxWorldIndex = floorf(coordinate + extent);
		return glm::ivec2(getChunkIndex(minWorldIndex, chunkSize), getChunkIndex(maxWorldIndex, chunkSize));
	}

	void World::loadSurroundingChunks(const glm::vec3& position, uint32_t radius)
	{
		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();

		auto boundariesX = getMinMaxChunkOffsets(position.x, radius, Chunk::MaxWidth);
		auto boundariesZ = getMinMaxChunkOffsets(position.z, radius, Chunk::MaxLength);

		for (int x = boundariesX[0]; x <= boundariesX[1]; x++) {
			for (int z = boundariesZ[0]; z <= boundariesZ[1]; z++) {
				auto key = getChunkKey(x, z);
				if (chunks.find(key) == chunks.end()) {
					loadChunk(x, z);
				}
			}
		}

		if (terrain.getChunks().size() > maxChunksInMemory) {
			unloadDistantChunks(position, purgeRemainingChunksRadius);
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
		
		terrain.addToRebuildWithAdjacent(chunkKey);
		chunks[chunkKey] = chunk;
	}

	void World::unloadDistantChunks(const glm::vec3& position, uint32_t remainingRadius)
	{
		auto boundariesX = getMinMaxChunkOffsets(position.x, remainingRadius, Chunk::MaxWidth);
		auto boundariesZ = getMinMaxChunkOffsets(position.z, remainingRadius, Chunk::MaxLength);

		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();
		std::set<uint64_t>& dirtyChunkKeys = terrain.getChunkKeysToSave();

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

	World::~World()
	{
	}
}