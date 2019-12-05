#include "SinglePlayerGame.h"

namespace yamc
{
	SinglePlayerGame::SinglePlayerGame(int seed, const std::string& worldName, uint32_t visibleChunksRadius) :
		Game(visibleChunksRadius),
		worldDataManager(seed, worldName)
	{
	}

	void SinglePlayerGame::init()
	{
		Game::init();
	}

	void SinglePlayerGame::update(const glm::vec3& playerPosition, float dt)
	{
		Game::update(playerPosition, dt);
	}

	Chunk* SinglePlayerGame::loadChunk(uint64_t key)
	{
		return worldDataManager.loadChunk(key);
	}

	void SinglePlayerGame::saveChunk(uint64_t key, Chunk* chunk)
	{
		worldDataManager.saveChunk(key, chunk);
	}

	void SinglePlayerGame::destroy()
	{
		Game::destroy();

		auto chunks = terrain.getChunks();
		for (uint64_t key : terrain.getChunkKeysToSave()) {
			worldDataManager.saveChunk(key, chunks[key]);
		}
	}
}