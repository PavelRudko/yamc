#ifndef YAMC_COMPRESSION_H
#define YAMC_COMPRESSION_H

#include <stdint.h>
#include <vector>

namespace yamc
{
	void compressData(const void* src, size_t size, std::vector<uint8_t>& dst);
	void decompressData(const void* src, size_t size, void* dst, size_t dstSize);
}

#endif
