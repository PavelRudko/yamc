#ifndef YAMC_GAME_H
#define YAMC_GAME_H

#include "Terrain.h"
#include "Entities.h"
#include "WorldDataManager.h"
#include "Renderer.h"
#include <queue>
#include <thread>
#include <mutex>

namespace yamc
{
	class Game
	{
	public:
		Game(uint32_t visibleChunksRadius);

		virtual void init();

		void setVisibleChunkRadius(uint32_t visibleChunksRadius);
		virtual void update(const glm::vec3& playerPosition, float dt);
		static void backgroundUpdateLoop(Game* game);

		virtual void loadSurroundingChunks(const glm::vec3& playerPosition);

		const Terrain& getTerrain() const;
		Terrain& getTerrain();

		const std::unordered_map<uint64_t, Mesh*>& getChunkMeshes() const;
		void rebuildChunkMeshes(Renderer* renderer);

		virtual void destroy();

	protected:
		Terrain terrain;
		uint32_t minSurroundingChunksRadius;
		uint32_t purgeRemainingChunksRadius;
		uint32_t maxChunksInMemory;
		std::thread backgroundThread;

		void requestSurroundingChunks(const glm::vec3& playerPosition);
		void unloadDistantChunks(const glm::vec3& position);

		virtual Chunk* loadChunk(uint64_t key) = 0;
		virtual void saveChunk(uint64_t key, Chunk* chunk) = 0;

	private:
		bool isRunning;
		std::unordered_map<uint64_t, Mesh*> chunkMeshes;
		std::queue<uint64_t> chunkKeysToLoad;
		std::mutex chunksToLoadMutex;

		void backgroundUpdate();
	};
}

#endif
