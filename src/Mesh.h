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
}

#endif