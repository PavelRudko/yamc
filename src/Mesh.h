#ifndef YAMC_MESH_H
#define YAMC_MESH_H

#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>

namespace yamc
{
	class Mesh
	{
	public:
		Mesh();
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&& other) noexcept;

		void setData(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices);
		void setData(const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& uvs, const std::vector<uint32_t>& indices);
		void setData(const std::vector<glm::vec4>& positions, const std::vector<glm::vec2>& uvs, const std::vector<uint32_t>& indices);

		uint32_t getVAO() const;
		uint32_t getIndicesCount() const;

		~Mesh();

	private:
		uint32_t vaoID;
		uint32_t vertexBufferID;
		uint32_t indexBufferID;
		uint32_t indicesCount;
	};

	const extern int BlockAtlasIndicesByType[4][3];
	const extern glm::vec3 BlockFaceVertices[6][4];
	const extern glm::ivec3 BlockFaceNormals[6];
	const extern float BlockFaceIlluminance[6];
	const extern float BlockWidth;
	const extern float BlockHalfWidth;

	void addBlockFace(std::vector<glm::vec4>& positions, std::vector<glm::vec2>& uvs, std::vector<uint32_t>& indices, const glm::vec3& center, uint32_t id, uint32_t face);
}

#endif