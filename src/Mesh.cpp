#include "Mesh.h"
#include <gl3w.h>

namespace yamc
{
	Mesh::Mesh() :
		indicesCount(0)
	{
		glGenVertexArrays(1, &vaoID);
		glGenBuffers(1, &vertexBufferID);
		glGenBuffers(1, &indexBufferID);
	}

	Mesh::Mesh(Mesh&& other) noexcept
	{
		indicesCount = other.indicesCount;
		vaoID = other.vaoID;
		vertexBufferID = other.vertexBufferID;
		indexBufferID = other.indexBufferID;
		other.vaoID = 0;
		other.vertexBufferID = 0;
		other.indexBufferID = 0;
	}

	void Mesh::setData(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices)
	{
		glBindVertexArray(vaoID);

		size_t positionsSize = positions.size() * sizeof(glm::vec3);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, positionsSize, nullptr, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, positions.data());

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);

		indicesCount = indices.size();
	}

	void Mesh::setData(const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& uvs, const std::vector<uint32_t>& indices)
	{
		glBindVertexArray(vaoID);

		size_t positionsSize = positions.size() * sizeof(glm::vec3);
		size_t uvsSize = uvs.size() * sizeof(glm::vec2);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, positionsSize + uvsSize, nullptr, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, positions.data());
		glBufferSubData(GL_ARRAY_BUFFER, positionsSize, uvsSize, uvs.data());

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)positionsSize);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);

		indicesCount = indices.size();
	}

	uint32_t Mesh::getVAO() const
	{
		return vaoID;
	}

	uint32_t Mesh::getIndicesCount() const
	{
		return indicesCount;
	}

	Mesh::~Mesh()
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