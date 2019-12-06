#include "Terrain.h"
#include <glm/gtc/constants.hpp>
#include <inttypes.h>

namespace yamc
{
	float fixedRandom(int x, int y, int seed)
	{
		x = x + y * 57 + seed;
		x = (x << 13) ^ x;
		return (1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
	}

	glm::vec2 randomDirection(int x, int y, int seed) 
	{
		float d = fixedRandom(x, y, seed) * glm::pi<float>();
		return glm::vec2(cosf(d), sinf(d));
	}

	float smoothStep(float x)
	{
		return x * x * x * (x * (x * 6 - 15) + 10);
	}

	float interpolateSmooth(float a, float b, float t)
	{
		t = smoothStep(t);
		return (1.0f - t) * a + t * b;
	}

	float perlinInterpolation(const glm::vec2& point, const glm::vec2& topLeft, const glm::vec2& topRight, const glm::vec2& bottomLeft, const glm::vec2& bottomRight)
	{
		float tl = glm::dot(point, topLeft);
		float tr = glm::dot(point - glm::vec2(1, 0), topRight);
		float bl = glm::dot(point - glm::vec2(0, 1), bottomLeft);
		float br = glm::dot(point - glm::vec2(1, 1), bottomRight);

		float top = interpolateSmooth(tl, tr, point.x);
		float bottom = interpolateSmooth(bl, br, point.x);

		return interpolateSmooth(top, bottom, point.y);
	}

	float perlin(float x, float y, int seed) 
	{
		int minX = (int)floorf(x);
		int maxX = minX + 1;
		int minY = (int)floorf(y);
		int maxY = minY + 1;

		auto topLeft = randomDirection(minX, minY, seed);
		auto topRight = randomDirection(maxX, minY, seed);
		auto bottomLeft = randomDirection(minX, maxY, seed);
		auto bottomRight = randomDirection(maxX, maxY, seed);

		glm::vec2 point(x - minX, y - minY);
		return perlinInterpolation(point, topLeft, topRight, bottomLeft, bottomRight);
	}

	float octavePerlin(float x, float y, int seed, int octaves)
	{
		float value = 0.0f;
		for (int i = 0; i < octaves; i++) {
			float s = powf(2.0f, i);
			value += (1.0 / s) * perlin(x * s, y * s, seed);
 		}
		return glm::clamp(value, -1.0f, 1.0f);
	}

	void fillChunk(Chunk* chunk, int offsetX, int offsetZ, int seed)
	{
		int maxHeight = 64;
		int minHeight = 32;
		float gridSize = 64.0f;

		for (int x = 0; x < Chunk::MaxWidth; x++) {
			for (int z = 0; z < Chunk::MaxLength; z++) {
				int worldX = x + offsetX * Chunk::MaxWidth;
				int worldZ = z + offsetZ * Chunk::MaxLength;

				float noise = octavePerlin(worldX / gridSize, worldZ / gridSize, seed, 3);
				int height = minHeight + (maxHeight - minHeight) * (0.5f + noise * 0.5f);
				for (int y = 0; y <= height; y++) {
					chunk->setBlock(x, y, z, y == height ? 2 : 1);
				}
			}
		}
	}

	uint64_t pack(uint32_t high, uint32_t low)
	{
		return (((uint64_t)high) << 32) | ((uint64_t)low);
	}

	uint32_t high(uint64_t key)
	{
		return key >> 32;
	}

	uint32_t low(uint64_t key)
	{
		return (std::numeric_limits<uint32_t>::max)() & key;
	}

	uint64_t getChunkKey(int high, int low)
	{
		return pack(high, low);
	}

	glm::ivec2 getChunkOffset(uint64_t key)
	{
		return glm::ivec2((int32_t)high(key), (int32_t)low(key));
	}

	int getChunkIndex(int worldIndex, int chunkSize)
	{
		if (worldIndex < 0) {
			return (worldIndex + 1) / chunkSize - 1;
		}
		return worldIndex / chunkSize;
	}

	uint32_t getLocalBlockIndex(int worldIndex, uint32_t chunkSize)
	{
		int localIndex = worldIndex % chunkSize;
		return localIndex >= 0 ? localIndex : chunkSize - localIndex - 1;
	}
			
	Terrain::Terrain()
	{
	}

	bool Terrain::findBlock(Chunk** outChunk, uint32_t& localX, uint32_t& localZ, uint64_t& chunkKey, int x, int y, int z) const
	{
		if (y < 0 || y >= Chunk::MaxHeight) {
			return false;
		}

		int32_t chunkX = getChunkIndex(x, Chunk::MaxWidth);
		int32_t chunkZ = getChunkIndex(z, Chunk::MaxLength);

		uint64_t key = getChunkKey(chunkX, chunkZ);
		auto chunk = chunks.find(key);
		if (chunk == chunks.end()) {
			return false;
		}

		chunkKey = key;
		localX = getLocalBlockIndex(x, Chunk::MaxWidth);
		localZ = getLocalBlockIndex(z, Chunk::MaxLength);
		*outChunk = chunk->second;

		return true;
	}

	uint32_t Terrain::getBlock(int x, int y, int z) const
	{	
		Chunk* chunk = nullptr;
		uint32_t localX, localZ;
		uint64_t chunkKey;
		if (!findBlock(&chunk, localX, localZ, chunkKey, x, y, z)) {
			return 0;
		}

		return chunk->getBlock(localX, y, localZ);
	}

	void Terrain::setBlock(int x, int y, int z, uint32_t id)
	{
		Chunk* chunk = nullptr;
		uint32_t localX, localZ;
		uint64_t chunkKey;
		if (!findBlock(&chunk, localX, localZ, chunkKey, x, y, z)) {
			return;
		}

		chunk->setBlock(localX, y, localZ, id);

		chunkKeysToSave.insert(chunkKey);
		
		auto chunkOffset = getChunkOffset(chunkKey);
		chunkKeysToRebuild.insert(chunkKey);
		if (localX == 0) {
			addToRebuildIfExists(chunkOffset[0] - 1, chunkOffset[1]);
		}
		if (localX == Chunk::MaxWidth - 1) {
			addToRebuildIfExists(chunkOffset[0] + 1, chunkOffset[1]);
		}
		if (localZ == 0) {
			addToRebuildIfExists(chunkOffset[0], chunkOffset[1] - 1);
		}
		if (localZ == Chunk::MaxLength - 1) {
			addToRebuildIfExists(chunkOffset[0], chunkOffset[1] + 1);
		}
	}

	const std::unordered_map<uint64_t, Chunk*>& Terrain::getChunks() const
	{
		return chunks;
	}

	std::unordered_map<uint64_t, Chunk*>& Terrain::getChunks()
	{
		return chunks;
	}

	const std::set<uint64_t>& Terrain::getChunkKeysToSave() const
	{
		return chunkKeysToSave;
	}

	std::set<uint64_t>& Terrain::getChunkKeysToSave()
	{
		return chunkKeysToSave;
	}

	const std::set<uint64_t>& Terrain::getChunkKeysToRebuild() const
	{
		return chunkKeysToRebuild;
	}

	std::set<uint64_t>& Terrain::getChunkKeysToRebuild()
	{
		return chunkKeysToRebuild;
	}

	void Terrain::addToRebuildWithAdjacent(uint64_t key)
	{
		chunkKeysToRebuild.insert(key);
		auto offset = getChunkOffset(key);
		for (int x = -1; x <= 1; x++) {
			for (int z = -1; z <= 1; z++) {
				if (abs(x) != abs(z)) {
					addToRebuildIfExists(offset[0] + x, offset[1] + z);
				}
			}
		}
	}

	void Terrain::addToRebuildIfExists(int x, int z)
	{
		uint64_t key = getChunkKey(x, z);
		if (chunks.find(key) != chunks.end()) {
			chunkKeysToRebuild.insert(key);
		}
	}

	Terrain::~Terrain()
	{
		for (const auto& pair : chunks) {
			delete pair.second;
		}
	}
}