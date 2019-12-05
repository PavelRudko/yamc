#ifndef YAMC_MESH_BUILDER
#define YAMC_MESH_BUILDER

#include "Texture.h"
#include <glm/glm.hpp>
#include <vector>
#include "Chunk.h"
#include "Terrain.h"
#include "Mesh.h"

namespace yamc
{
	class MeshBuilder
	{
	public:
		static const int BlockAtlasIndicesByType[4][3];

		MeshBuilder(const AtlasTexture& atlas);

		void addCubeBlock(std::vector<glm::vec4>& positions, 
			std::vector<glm::vec2>& uvs, 
			std::vector<uint32_t>& indices, 
			const glm::vec3 center, 
			const glm::ivec3 worldCoordinate, 
			uint32_t id, 
			const Terrain* terrain = nullptr) const;

		void rebuildChunk(uint64_t chunkKey, Chunk* chunk, const Terrain* terrain, Mesh* mesh) const;

	private:
		static const glm::ivec3 BlockFaceNormals[6];
		static const glm::vec3 BlockFaceVertices[6][4];
		static const float BlockFaceIlluminance[6];
		static const float BlockWidth;
		static const float BlockHalfWidth;

		const AtlasTexture& atlas;
	};
}

#endif