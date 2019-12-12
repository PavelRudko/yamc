#include "Compression.h"
#include <zlib.h>
#include <stdexcept>

namespace yamc
{
	const size_t TmpBufferSize = 128 * 1024;

	void compressData(const void* src, size_t size, std::vector<uint8_t>& dst)
	{
		std::vector<uint8_t> buffer;
		static uint8_t tmpBuffer[TmpBufferSize];

		z_stream stream;
		stream.zalloc = 0;
		stream.zfree = 0;
		stream.next_in = (uint8_t*)src;
		stream.avail_in = size;
		stream.next_out = tmpBuffer;
		stream.avail_out = TmpBufferSize;

		deflateInit(&stream, Z_BEST_COMPRESSION);
		while (stream.avail_in != 0) {
			int result = deflate(&stream, Z_NO_FLUSH);
			if (result != Z_OK) {
				throw std::runtime_error("Error during compression.");
			}

			if (stream.avail_out == 0) {
				buffer.insert(buffer.end(), tmpBuffer, tmpBuffer + TmpBufferSize);
				stream.next_out = tmpBuffer;
				stream.avail_out = TmpBufferSize;
			}
		}

		int result = Z_OK;
		while (result == Z_OK) {
			if (stream.avail_out == 0) {
				buffer.insert(buffer.end(), tmpBuffer, tmpBuffer + TmpBufferSize);
				stream.next_out = tmpBuffer;
				stream.avail_out = TmpBufferSize;
			}

			result = deflate(&stream, Z_FINISH);
		}

		if (result != Z_STREAM_END) {
			throw std::runtime_error("Error during compression.");
		}

		buffer.insert(buffer.end(), tmpBuffer, tmpBuffer + TmpBufferSize - stream.avail_out);
		deflateEnd(&stream);

		dst.swap(buffer);
	}

	void decompressData(const void* src, size_t size, void* dst, size_t dstSize)
	{
		uLongf dstDizeZ = dstSize;
		uncompress((uint8_t*)dst, &dstDizeZ, (const uint8_t*)src, size);
	}
}