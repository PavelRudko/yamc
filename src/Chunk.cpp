#include "Chunk.h"
#include <gl3w.h>
#include <vector>
#include <algorithm>

namespace yamc
{
	Chunk::Chunk() :
		indicesCount(0)
	{
		glGenVertexArrays(1, &vaoID);
		glGenBuffers(1, &vertexBufferID);
		glGenBuffers(1, &indexBufferID);

		memset(blocks, 0, MaxWidth * MaxHeight * MaxLength * sizeof(uint32_t));
	}

	uint32_t Chunk::getVAO() const
	{
		return vaoID;
	}

	uint32_t Chunk::getIndicesCount() const
	{
		return indicesCount;
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

	void Chunk::update()
	{
		std::vector<glm::vec4> positions;
		std::vector<glm::vec2> uvs;
		std::vector<uint32_t> indices;

		for (int x = 0; x < MaxWidth; x++) {
			for (int y = 0; y < MaxHeight; y++) {
				for (int z = 0; z < MaxLength; z++) {
					uint32_t id = blocks[x][y][z];
					if (id > 0) {

						glm::vec3 center(x, y, z);
						center *= BlockWidth;
						center += glm::vec3(BlockHalfWidth, BlockHalfWidth, BlockHalfWidth);

						for (int f = 0; f < 6; f++) {
							if (isFaceVisible(x, y, z, BlockFaceNormals[f])) {
								addBlockFace(positions, uvs, indices, center, id, f);
							}
						}
					}
				}
			}
		}

		indicesCount = indices.size();
		
		glBindVertexArray(vaoID);

		size_t positionsSize = positions.size() * sizeof(glm::vec4);
		size_t uvsSize = uvs.size() * sizeof(glm::vec2);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, positionsSize + uvsSize, nullptr, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, positions.data());
		glBufferSubData(GL_ARRAY_BUFFER, positionsSize, uvsSize, uvs.data());

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)positionsSize);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	uint32_t Chunk::getBlock(uint32_t x, uint32_t y, uint32_t z) const
	{
		return blocks[x][y][z];
	}

	void Chunk::setBlock(uint32_t x, uint32_t y, uint32_t z, uint32_t value)
	{
		blocks[x][y][z] = value;
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
		if (vaoID > 0) {
			glDeleteVertexArrays(1, &vaoID);
		}

		if (vertexBufferID > 0) {
			glDeleteBuffers(1, &vertexBufferID);
		}

		if (indexBufferID > 0) {
			glDeleteBuffers(1, &indexBufferID);
		}
	}
}