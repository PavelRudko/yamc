#include "WorldDataManager.h"
#include <filesystem>

namespace yamc
{
	WorldDataManager::WorldDataManager(int seed, const std::string& worldName) :
		seed(seed),
		directoryPath("worlds/" + worldName)
	{
		ensureWorldDirectoryExists();
	}

	Chunk* WorldDataManager::loadChunk(uint64_t key)
	{
		auto chunk = new Chunk();

		FILE* file = fopen(getChunkPath(key).c_str(), "rb");
		if (file) {
			fread(chunk->getData(), sizeof(uint32_t), Chunk::MaxHeight * Chunk::MaxLength * Chunk::MaxWidth, file);
			fclose(file);
		}
		else {
			auto offset = getChunkOffset(key);
			fillChunk(chunk, offset[0], offset[1], seed);
		}

		return chunk;
	}

	void WorldDataManager::saveChunk(uint64_t key, const Chunk* chunk) const
	{
		auto path = getChunkPath(key);
		FILE* file = fopen(path.c_str(), "wb");
		fwrite(chunk->getData(), sizeof(uint32_t), Chunk::MaxHeight * Chunk::MaxLength * Chunk::MaxWidth, file);
		fclose(file);
	}

	std::string WorldDataManager::getChunkPath(uint64_t key) const
	{
		return directoryPath + std::to_string(key) + ".cnk";
	}

	void WorldDataManager::ensureWorldDirectoryExists() const
	{
		std::string worldsRootDirectory("worlds");
		if (!std::filesystem::is_directory(worldsRootDirectory) || !std::filesystem::exists(worldsRootDirectory)) {
			std::filesystem::create_directory(worldsRootDirectory);
		}

		if (!std::filesystem::is_directory(directoryPath) || !std::filesystem::exists(directoryPath)) {
			std::filesystem::create_directory(directoryPath);
		}
	}
}