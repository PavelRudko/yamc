#include "MeshBuilder.h"
#include <algorithm>

namespace yamc
{
	const int MeshBuilder::BlockAtlasIndicesByType[4][3] =
	{
		{0, 0, 0},
		{2, 2, 2},
		{1, 2, 0},
		{4, 4, 3}
	};

	const glm::ivec3 MeshBuilder::BlockFaceNormals[6] = 
	{
		{0, 1, 0}, //top
		{0, -1, 0}, //bottom
		{0, 0, 1}, //front
		{0, 0, -1}, //back
		{1, 0, 0}, //right
		{-1, 0, 0} //left
	};

	const glm::vec3 MeshBuilder::BlockFaceVertices[6][4] =
	{
		{ {-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1} }, //top
		{ {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1} }, //bottom
		{ {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1} }, //front
		{ {1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1} }, //back
		{ {1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1} }, //right
		{ {-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1} } //left
	};

	const float MeshBuilder::BlockFaceIlluminance[6] =
	{
		1.0f,
		0.1f,
		0.5f,
		0.8f,
		0.7f,
		0.4f
	};

	const float MeshBuilder::BlockWidth = 1.0f;
	const float MeshBuilder::BlockHalfWidth = BlockWidth / 2;

	MeshBuilder::MeshBuilder(const AtlasTexture& atlas) :
		atlas(atlas)
	{
	}

	void MeshBuilder::addCubeBlock(std::vector<glm::vec4>& positions, std::vector<glm::vec2>& uvs, std::vector<uint32_t>& indices, const glm::vec3& center, uint32_t id, uint32_t* surroundingBlocks) const
	{
		for (int face = 0; face < 6; face++) {
			if (surroundingBlocks == nullptr || surroundingBlocks[face] == 0) {
				const int* atlasIndices = id < sizeof(BlockAtlasIndicesByType) ? BlockAtlasIndicesByType[id] : BlockAtlasIndicesByType[0];

				uint32_t baseIndex = positions.size();
				float illuminance = BlockFaceIlluminance[face];

				for (int i = 0; i < 4; i++) {
					auto position = glm::vec3(center + BlockFaceVertices[face][i] * BlockHalfWidth);
					positions.push_back(glm::vec4(position, illuminance));
				}

				indices.push_back(baseIndex + 0);
				indices.push_back(baseIndex + 1);
				indices.push_back(baseIndex + 2);
				indices.push_back(baseIndex + 0);
				indices.push_back(baseIndex + 2);
				indices.push_back(baseIndex + 3);

				int atlasIndex = atlasIndices[(std::min)(face, 2)];
				auto uvMin = atlas.getTileOffset(atlasIndex);
				auto uvMax = uvMin + atlas.getUVScale();

				uvs.push_back({ uvMin.x, uvMax.y });
				uvs.push_back({ uvMax.x, uvMax.y });
				uvs.push_back({ uvMax.x, uvMin.y });
				uvs.push_back({ uvMin.x, uvMin.y });
			}
		}
	}

	void MeshBuilder::rebuildChunk(Chunk* chunk) const
	{
		static uint32_t surroundingBlocks[6];

		std::vector<glm::vec4> positions;
		std::vector<glm::vec2> uvs;
		std::vector<uint32_t> indices;

		for (int x = 0; x < Chunk::MaxWidth; x++) {
			for (int y = 0; y < Chunk::MaxHeight; y++) {
				for (int z = 0; z < Chunk::MaxLength; z++) {
					uint32_t id = chunk->getBlock(x, y, z);
					if (id > 0) {

						for (int i = 0; i < 6; i++) {
							auto coord = BlockFaceNormals[i];
							coord.x += x;
							coord.y += y;
							coord.z += z;
							if (coord.x >= 0 && coord.x < Chunk::MaxWidth &&
								coord.y >= 0 && coord.y < Chunk::MaxHeight &&
								coord.z >= 0 && coord.z < Chunk::MaxLength) {
								surroundingBlocks[i] = chunk->getBlock(coord.x, coord.y, coord.z);
							}
							else {
								surroundingBlocks[i] = 0;
							}
						}

						glm::vec3 center(x, y, z);
						center *= BlockWidth;
						center += glm::vec3(BlockHalfWidth, BlockHalfWidth, BlockHalfWidth);

						addCubeBlock(positions, uvs, indices, center, id, surroundingBlocks);
					}
				}
			}
		}

		chunk->getMesh().setData(positions, uvs, indices);
	}
};

