#ifndef YAMC_NETWORK_H
#define YAMC_NETWORK_H

#include <stdint.h>

namespace yamc
{
	struct BlockDiff
	{
		int x, y, z;
		uint32_t type;
	};

	enum class RequestCodes : uint8_t
	{
		UpdateBlockDiffs = 1
	};

	enum class ResponseCodes : uint8_t
	{
		UpdateBlockDiffs = 1
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

		void writeByte(uint8_t value);
		void writeInt32(int32_t value);
		void writeUint32(uint32_t value);
		void writeFloat(float value);

		uint8_t readByte();
		int32_t readInt32();
		uint32_t readUint32();
		float readFloat();

		void rewind();

		~PackageBuffer();

	private:
		uint8_t* data;
		size_t offset;
		size_t capacity;
	};
}

#endif