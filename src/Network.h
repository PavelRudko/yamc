#ifndef YAMC_NETWORK_H
#define YAMC_NETWORK_H

#include <stdint.h>
#include <queue>

namespace yamc
{
	struct BlockDiff
	{
		int x, y, z;
		uint32_t type;
	};

	enum class RequestCodes : uint8_t
	{
		Connect = 0,
		UpdateBlockDiffs = 1,
		LoadChunk = 2
	};

	enum class ResponseCodes : uint8_t
	{
		WorldSettings = 0,
		UpdateBlockDiffs = 1,
		ChunkIsUnchanged = 2,
		ChunkDataStart = 3,
		ChunkDataPart = 4
	};

	constexpr int PackageBufferCapacity = 1024;

	class PackageBuffer
	{
	public:
		PackageBuffer(size_t capacity);

		PackageBuffer(const PackageBuffer&) = delete;
		PackageBuffer(PackageBuffer&& other) noexcept;

		const void* getData() const;
		void* getData();
		size_t getOffset() const;
		size_t getCapacity() const;
		size_t getAvailableSpace() const;

		void writeByte(uint8_t value);
		void writeInt32(int32_t value);
		void writeUint32(uint32_t value);
		void writeUint64(uint64_t value);
		void writeFloat(float value);
		void writeBuffer(const void* src, size_t size);

		uint8_t readByte();
		int32_t readInt32();
		uint32_t readUint32();
		uint64_t readUint64();
		float readFloat();
		void readBuffer(void* dst, size_t size);

		void rewind();

		~PackageBuffer();

	private:
		uint8_t* data;
		size_t offset;
		size_t capacity;
	};

	void writeBlockDiffsToPackage(PackageBuffer& packageBuffer, std::queue<BlockDiff>& blockDiffs);
	void readBlockDiffsFromPackage(PackageBuffer& packageBuffer, std::vector<BlockDiff>& blockDiffs);
}

#endif