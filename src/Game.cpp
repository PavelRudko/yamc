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
		backgroundThread = std::move(std::thread(backgroundUpdate, this));
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

	void Game::backgroundUpdate(Game* game)
	{
		while (game->isRunning) {
			if (game->chunkKeysToLoad.empty()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			std::unordered_map<uint64_t, Chunk*>& chunks = game->terrain.getChunks();

			while (!game->chunkKeysToLoad.empty()) {
				uint64_t chunkKey = game->chunkKeysToLoad.front();
				auto chunk = game->loadChunk(chunkKey);
				chunks[chunkKey] = chunk;
				game->terrain.addToRebuildWithAdjacent(chunkKey);
				game->chunkKeysToLoad.pop();
			}
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

	void Game::destroy()
	{
		isRunning = false;
		backgroundThread.join();

		for (auto pair : chunkMeshes) {
			delete pair.second;
		}
	}
}