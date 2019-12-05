#ifndef YAMC_SINGLEPLAYERGAME_H
#define YAMC_SINGLEPLAYERGAME_H

#include "Game.h"

namespace yamc
{
	class SinglePlayerGame : public Game
	{
	public:
		SinglePlayerGame(int seed, const std::string& worldName, uint32_t visibleChunksRadius);
		virtual void init() override;
		virtual void update(const glm::vec3& playerPosition, float dt) override;
		virtual void destroy() override;

	protected:
		virtual Chunk* loadChunk(uint64_t key) override;
		virtual void saveChunk(uint64_t key, Chunk* chunk) override;

	private:
		WorldDataManager worldDataManager;
	};
}

#endif