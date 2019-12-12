#ifndef YAMC_MULTIPLAYER_GAME_H
#define YAMC_MULTIPLAYER_GAME_H

#include "Game.h"
#include "Network.h"
#include "Sockets.h"
#include <atomic>

namespace yamc
{
	class MultiPlayerGame : public Game
	{
	public:
		MultiPlayerGame(uint32_t visibleChunksRadius);
		virtual void init() override;
		virtual void update(const glm::vec3& playerPosition, float dt) override;
		virtual void destroy() override;
		virtual void loadSurroundingChunks(const glm::vec3& playerPosition) override;

	protected:
		virtual Chunk* loadChunk(uint64_t key) override;
		virtual void saveChunk(uint64_t key, Chunk* chunk) override;
		virtual void backgroundUpdate() override;
		virtual void setBlock(int x, int y, int z, uint32_t type) override;

	private:
		int seed;
		SOCKET sock;
		std::atomic_bool isConnected;
		PackageBuffer packageBuffer;
		std::queue<BlockDiff> blockDiffsToSend;
		std::mutex blockDiffsToSendMutex;
		std::mutex connectionMutex;

		bool requestConnection();
		void updateBlockDiffs();
		bool receivePackage(PackageBuffer& packageBuffer);
		bool sendPackage(PackageBuffer& packageBuffer);
	};
}

#endif