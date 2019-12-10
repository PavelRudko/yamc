#ifndef YAMC_MULTIPLAYER_GAME_H
#define YAMC_MULTIPLAYER_GAME_H

#include "Game.h"
#include "Network.h"
#include "Sockets.h"

namespace yamc
{
	class MultiPlayerGame : public Game
	{
	public:
		MultiPlayerGame(uint32_t visibleChunksRadius);
		virtual void init() override;
		virtual void update(const glm::vec3& playerPosition, float dt) override;
		virtual void destroy() override;

	protected:
		virtual Chunk* loadChunk(uint64_t key) override;
		virtual void saveChunk(uint64_t key, Chunk* chunk) override;
		virtual void backgroundUpdate() override;
		virtual void setBlock(int x, int y, int z, uint32_t type) override;

	private:
		WorldDataManager worldDataManager;
		SOCKET sock;
		PackageBuffer packageBuffer;
		std::queue<BlockDiff> blockDiffsToSend;
		std::mutex blockDiffsToSendMutex;

		void updateBlockDiffs();
	};
}

#endif