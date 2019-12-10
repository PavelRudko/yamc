#include "Game.h"
#include "Entities.h"

namespace yamc
{
	glm::ivec2 getMinMaxChunkOffsets(float coordinate, uint32_t radius, uint32_t chunkSize)
	{
		float extent = radius * chunkSize;
		int minWorldIndex = floorf(coordinate - extent);
		int maxWorldIndex = floorf(coordinate + extent);
		return glm::ivec2(getChunkIndex(minWorldIndex, chunkSize), getChunkIndex(maxWorldIndex, chunkSize));
	}

	Game::Game(uint32_t visibleChunksRadius) :
		backgroundThread(),
		isRunning(false)
	{
		setVisibleChunkRadius(visibleChunksRadius);
	}

	void Game::init()
	{
		isRunning = true;
		backgroundThread = std::move(std::thread(backgroundUpdateLoop, this));
	}

	void Game::setVisibleChunkRadius(uint32_t visibleChunksRadius)
	{
		minSurroundingChunksRadius = visibleChunksRadius + 1;
		purgeRemainingChunksRadius = visibleChunksRadius + 2;
		maxChunksInMemory = pow((purgeRemainingChunksRadius * 2 + 1), 2) * 3;
	}

	void Game::unloadDistantChunks(const glm::vec3& position)
	{
		auto boundariesX = getMinMaxChunkOffsets(position.x, purgeRemainingChunksRadius, Chunk::MaxWidth);
		auto boundariesZ = getMinMaxChunkOffsets(position.z, purgeRemainingChunksRadius, Chunk::MaxLength);

		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();
		std::set<uint64_t>& dirtyChunkKeys = terrain.getChunkKeysToSave();

		for (auto it = chunks.begin(); it != chunks.end();) {
			auto offset = getChunkOffset(it->first);
			if (offset[0] < boundariesX[0] || offset[0] > boundariesX[1] ||
				offset[1] < boundariesZ[0] || offset[1] > boundariesZ[1]) {
				auto chunk = it->second;
				if (dirtyChunkKeys.count(it->first) > 0) {
					saveChunk(it->first, chunk);
					dirtyChunkKeys.erase(it->first);
				}

				chunkMeshes.erase(it->first);
				it = chunks.erase(it);
				delete chunk;
			}
			else {
				it++;
			}
		}
	}

	void Game::update(const glm::vec3& playerPosition, float dt)
	{
		requestSurroundingChunks(playerPosition);
		if (terrain.getChunks().size() > maxChunksInMemory) {
			unloadDistantChunks(playerPosition);
		}
	}

	void Game::backgroundUpdate()
	{
		uint64_t chunkKey = 0;
		{
			std::lock_guard<std::mutex> guard(chunksToLoadMutex);
			if (chunkKeysToLoad.empty()) {
				return;
			}

			chunkKey = chunkKeysToLoad.front();
			chunkKeysToLoad.pop();
		}
		
		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();

		auto chunk = loadChunk(chunkKey);
		if (chunk == nullptr) {
			return;
		}

		chunks[chunkKey] = chunk;
		terrain.addToRebuildWithAdjacent(chunkKey);
	}

	void Game::backgroundUpdateLoop(Game* game)
	{
		while (game->isRunning) {
			game->backgroundUpdate();
		}
	}

	void Game::requestSurroundingChunks(const glm::vec3& playerPosition)
	{
		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();

		auto boundariesX = getMinMaxChunkOffsets(playerPosition.x, minSurroundingChunksRadius, Chunk::MaxWidth);
		auto boundariesZ = getMinMaxChunkOffsets(playerPosition.z, minSurroundingChunksRadius, Chunk::MaxLength);

		for (int x = boundariesX[0]; x <= boundariesX[1]; x++) {
			for (int z = boundariesZ[0]; z <= boundariesZ[1]; z++) {
				auto key = getChunkKey(x, z);
				if (chunks.find(key) == chunks.end()) {
					std::lock_guard<std::mutex> guard(chunksToLoadMutex);
					chunkKeysToLoad.push(key);
				}
			}
		}
	}

	void Game::loadSurroundingChunks(const glm::vec3& playerPosition)
	{
		std::unordered_map<uint64_t, Chunk*>& chunks = terrain.getChunks();

		auto boundariesX = getMinMaxChunkOffsets(playerPosition.x, minSurroundingChunksRadius, Chunk::MaxWidth);
		auto boundariesZ = getMinMaxChunkOffsets(playerPosition.z, minSurroundingChunksRadius, Chunk::MaxLength);

		for (int x = boundariesX[0]; x <= boundariesX[1]; x++) {
			for (int z = boundariesZ[0]; z <= boundariesZ[1]; z++) {
				auto key = getChunkKey(x, z);
				if (chunks.find(key) == chunks.end()) {
					auto chunk = loadChunk(key);
					terrain.addToRebuildWithAdjacent(key);
					chunks[key] = chunk;
				}
			}
		}
	}

	const Terrain& Game::getTerrain() const
	{
		return terrain;
	}

	Terrain& Game::getTerrain()
	{
		return terrain;
	}

	const std::unordered_map<uint64_t, Mesh*>& Game::getChunkMeshes() const
	{
		return chunkMeshes;
	}

	void Game::rebuildChunkMeshes(Renderer* renderer)
	{
		auto& chunksToRebuild = terrain.getChunkKeysToRebuild();
		for (uint64_t chunkKey : chunksToRebuild) {
			auto chunk = terrain.getChunks()[chunkKey];
			auto meshEntry = chunkMeshes.find(chunkKey);
			Mesh* chunkMesh = nullptr;

			if (meshEntry == chunkMeshes.end()) {
				chunkMesh = new Mesh();
				chunkMeshes[chunkKey] = chunkMesh;
			}
			else {
				chunkMesh = chunkMeshes[chunkKey];
			}

			renderer->getMeshBuilder().rebuildChunk(chunkKey, chunk, &terrain, chunkMesh);
		}
		chunksToRebuild.clear();
	}

	void Game::setBlock(int x, int y, int z, uint32_t type)
	{
		terrain.setBlock(x, y, z, type);
	}

	uint32_t Game::getBlock(int x, int y, int z) const
	{
		return terrain.getBlock(x, y, z);
	}

	void Game::destroy()
	{
		isRunning = false;
		backgroundThread.join();

		for (auto pair : chunkMeshes) {
			delete pair.second;
		}
	}
}