#include "Chunk.h"
#include <gl3w.h>
#include <vector>
#include <algorithm>

namespace yamc
{
	Chunk::Chunk()
	{
		memset(blocks, 0, MaxWidth * MaxHeight * MaxLength * sizeof(uint32_t));
	}

	bool Chunk::isFaceVisible(int x, int y, int z, const glm::ivec3& normal) const
	{
		x += normal.x;
		y += normal.y;
		z += normal.z;

		if (x < 0 || x >= MaxWidth || 
			y < 0 || y >= MaxHeight || 
			z < 0 || z >= MaxLength) {
			return true;
		}

		return blocks[x][y][z] == 0;
	}

	uint32_t Chunk::getBlock(uint32_t x, uint32_t y, uint32_t z) const
	{
		return blocks[x][y][z];
	}

	void Chunk::setBlock(uint32_t x, uint32_t y, uint32_t z, uint32_t value)
	{
		blocks[x][y][z] = value;
	}

	const Mesh& Chunk::getMesh() const
	{
		return mesh;
	}

	Mesh& Chunk::getMesh()
	{
		return mesh;
	}

	const void* Chunk::getData() const
	{
		return &blocks[0][0][0];
	}

	void* Chunk::getData()
	{
		return &blocks[0][0][0];
	}

	Chunk::~Chunk()
	{
	}
}