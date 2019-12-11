#include "Network.h"

namespace yamc
{
	PackageBuffer::PackageBuffer(size_t capacity) :
		capacity(capacity),
		offset(0)
	{
		data = new uint8_t[capacity];
	}

	PackageBuffer::PackageBuffer(PackageBuffer&& other) noexcept
	{
		capacity = other.capacity;
		data = other.data;
		offset = other.offset;
		other.data = nullptr;
	}

	const void* PackageBuffer::getData() const
	{
		return data;
	}

	void* PackageBuffer::getData()
	{
		return data;
	}

	size_t PackageBuffer::getOffset() const
	{
		return offset;
	}

	size_t PackageBuffer::getCapacity() const
	{
		return capacity;
	}

	void PackageBuffer::writeByte(uint8_t value)
	{
		*(data + offset) = value;
		offset++;
	}

	void PackageBuffer::writeInt32(int32_t value)
	{
#ifdef BIG_ENDIAN
		*((int32_t*)(data + offset)) = bswap(value);
#else
		*((int32_t*)(data + offset)) = value;
#endif
		offset += sizeof(int32_t);
	}

	void PackageBuffer::writeUint32(uint32_t value)
	{
#ifdef BIG_ENDIAN
		* ((uint32_t*)(data + offset)) = bswap(value);
#else
		* ((uint32_t*)(data + offset)) = value;
#endif
		offset += sizeof(uint32_t);
	}

	void PackageBuffer::writeFloat(float value)
	{
#ifdef BIG_ENDIAN
		*((float*)(data + offset)) = bswap(value);
#else
		*((float*)(data + offset)) = value;
#endif
		offset += sizeof(float);
	}

	uint8_t PackageBuffer::readByte()
	{
		uint8_t value = *(data + offset);
		offset++;
		return value;
	}

	int32_t PackageBuffer::readInt32()
	{	
#ifdef BIG_ENDIAN
		int32_t value = *((int32_t*)(data + offset)) = bswap(value);
#else
		int32_t value = *((int32_t*)(data + offset));
#endif
		offset += sizeof(int32_t);
		return value;
	}

	uint32_t PackageBuffer::readUint32()
	{
#ifdef BIG_ENDIAN
		uint32_t value = *((uint32_t*)(data + offset)) = bswap(value);
#else
		uint32_t value = *((uint32_t*)(data + offset));
#endif
		offset += sizeof(uint32_t);
		return value;
	}

	float PackageBuffer::readFloat()
	{
#ifdef BIG_ENDIAN
		int32_t value = *((float*)(data + offset)) = bswap(value);
#else
		float value = *((float*)(data + offset));
#endif
		offset += sizeof(float);
		return value;
	}

	void PackageBuffer::rewind()
	{
		offset = 0;
	}

	PackageBuffer::~PackageBuffer()
	{
		if (data) {
			delete data;
		}
	}

	void writeBlockDiffsToPackage(PackageBuffer& packageBuffer, std::queue<BlockDiff>& blockDiffs)
	{
		packageBuffer.writeByte(blockDiffs.size());
		while (!blockDiffs.empty()) {
			auto diff = blockDiffs.front();
			blockDiffs.pop();

			packageBuffer.writeInt32(diff.x);
			packageBuffer.writeInt32(diff.y);
			packageBuffer.writeInt32(diff.z);
			packageBuffer.writeUint32(diff.type);
		}
	}

	void readBlockDiffsFromPackage(PackageBuffer& packageBuffer, std::vector<BlockDiff>& blockDiffs)
	{
		uint8_t count = packageBuffer.readByte();
		for (int i = 0; i < count; i++) {
			BlockDiff blockDiff;
			blockDiff.x = packageBuffer.readInt32();
			blockDiff.y = packageBuffer.readInt32();
			blockDiff.z = packageBuffer.readInt32();
			blockDiff.type = packageBuffer.readUint32();
			blockDiffs.push_back(blockDiff);
		}
	}
}