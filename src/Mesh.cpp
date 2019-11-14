#include "Mesh.h"
#include <gl3w.h>
#include <algorithm>

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

	void Mesh::setData(const std::vector<glm::vec4>& positions, const std::vector<glm::vec2>& uvs, const std::vector<uint32_t>& indices)
	{
		glBindVertexArray(vaoID);

		size_t positionsSize = positions.size() * sizeof(glm::vec4);
		size_t uvsSize = uvs.size() * sizeof(glm::vec2);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, positionsSize + uvsSize, nullptr, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, positions.data());
		glBufferSubData(GL_ARRAY_BUFFER, positionsSize, uvsSize, uvs.data());

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
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

	const int BlockAtlasIndicesByType[4][3] = 
	{
		{0, 0, 0},
		{2, 2, 2},
		{1, 2, 0},
		{4, 4, 3}
	};

	const glm::vec3 BlockFaceVertices[6][4] =
	{
		{ {-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1} }, //top
		{ {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1} }, //bottom
		{ {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1} }, //front
		{ {1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1} }, //back
		{ {1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1} }, //right
		{ {-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1} } //left
	};

	const glm::ivec3 BlockFaceNormals[6] =
	{
		{0, 1, 0}, //top
		{0, -1, 0}, //bottom
		{0, 0, 1}, //front
		{0, 0, -1}, //back
		{1, 0, 0}, //right
		{-1, 0, 0} //left
	};

	const float BlockFaceIlluminance[6] =
	{
		1.0f,
		0.1f,
		0.5f,
		0.8f,
		0.7f,
		0.4f
	};

	const float BlockWidth = 1.0f;
	const float BlockHalfWidth = BlockWidth / 2.0f;

	void addBlockFace(std::vector<glm::vec4>& positions, std::vector<glm::vec2>& uvs, std::vector<uint32_t>& indices, const glm::vec3& center, uint32_t id, uint32_t face)
	{
		static int atlasPixelWidth = 512;
		static int atlasPixelHeight = 512;
		static int blockPixelWidth = 16;
		static int blockPixelHeight = 16;
		static int atlasBlockWidth = atlasPixelWidth / blockPixelWidth;

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

		int atlasIndex = atlasIndices[(std::min)(face, (uint32_t)2)];
		float uvMinX = (atlasIndex % atlasBlockWidth) * blockPixelWidth;
		float uvMinY = (atlasIndex / atlasBlockWidth) * blockPixelHeight;
		float uvMaxX = uvMinX + blockPixelWidth;
		float uvMaxY = uvMinY + blockPixelHeight;
		uvMinX /= atlasPixelWidth;
		uvMinY /= atlasPixelHeight;
		uvMaxX /= atlasPixelWidth;
		uvMaxY /= atlasPixelHeight;

		uvs.push_back({ uvMinX, uvMaxY });
		uvs.push_back({ uvMaxX, uvMaxY });
		uvs.push_back({ uvMaxX, uvMinY });
		uvs.push_back({ uvMinX, uvMinY });
	}
}