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

	int calculateAO(int side1, int side2, int corner)
	{
		if (side1 && side2) {
			return 0;
		}
		return 3 - (side1 + side2 + corner);
	}

	void MeshBuilder::addCubeBlock(std::vector<glm::vec4>& positions,
		std::vector<glm::vec2>& uvs,
		std::vector<uint32_t>& indices,
		const glm::vec3 center,
		const glm::ivec3 worldCoordinate,
		uint32_t id,
		const Terrain* terrain) const
	{
		for (int face = 0; face < 6; face++) {
			auto normal = BlockFaceNormals[face];
			auto adjuscentBlock = worldCoordinate + normal;
			if (terrain != nullptr && (adjuscentBlock.y < 0 || terrain->getBlock(adjuscentBlock.x, adjuscentBlock.y, adjuscentBlock.z) != 0)) {
				continue;
			}

			const int* atlasIndices = id < sizeof(BlockAtlasIndicesByType) ? BlockAtlasIndicesByType[id] : BlockAtlasIndicesByType[0];

			uint32_t baseIndex = positions.size();
			float illuminance = BlockFaceIlluminance[face];

			for (int i = 0; i < 4; i++) {
				auto vertexOffset = BlockFaceVertices[face][i];
				float vertexIlluminance = illuminance;
				if (terrain != nullptr) {
					auto a1 = worldCoordinate + glm::ivec3(vertexOffset.x, vertexOffset.y, 0);
					auto a2 = worldCoordinate + glm::ivec3(0, vertexOffset.y, vertexOffset.z);
					auto aCorner = worldCoordinate + glm::ivec3(vertexOffset.x, vertexOffset.y, vertexOffset.z);
					int side1 = terrain->getBlock(a1.x, a1.y, a1.z) != 0 ? 1 : 0;
					int side2 = terrain->getBlock(a2.x, a2.y, a2.z) != 0 ? 1 : 0;
					int corner = terrain->getBlock(aCorner.x, aCorner.y, aCorner.z) != 0 ? 1 : 0;
					vertexIlluminance += calculateAO(side1, side2, corner) * 0.1f;
					vertexIlluminance -= 0.3f;
				}
				
				auto position = glm::vec3(center + BlockFaceVertices[face][i] * BlockHalfWidth);
				positions.push_back(glm::vec4(position, vertexIlluminance));
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

	void MeshBuilder::rebuildChunk(uint64_t chunkKey, Chunk* chunk, const Terrain* terrain, Mesh* mesh) const
	{
		auto chunkOffset = getChunkOffset(chunkKey);

		static std::vector<glm::vec4> positions;
		static std::vector<glm::vec2> uvs;
		static std::vector<uint32_t> indices;

		positions.clear();
		uvs.clear();
		indices.clear();

		for (int x = 0; x < Chunk::MaxWidth; x++) {
			for (int y = 0; y < Chunk::MaxHeight; y++) {
				for (int z = 0; z < Chunk::MaxLength; z++) {
					uint32_t id = chunk->getBlock(x, y, z);
					if (id > 0) {
						glm::ivec3 worldCoordinate(x + chunkOffset[0] * Chunk::MaxWidth, y, z + chunkOffset[1] * Chunk::MaxLength);
						glm::vec3 center(x * BlockWidth + BlockHalfWidth, y * BlockWidth + BlockHalfWidth, z * BlockWidth + BlockHalfWidth);

						addCubeBlock(positions, uvs, indices, center, worldCoordinate, id, terrain);
					}
				}
			}
		}

		mesh->setData(positions, uvs, indices);
	}
};

