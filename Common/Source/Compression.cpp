#include <lzma.h>

#include <Common/Compression.hpp>
#include <Common/Debug.hpp>

void throwLzma(const std::string & message, lzma_ret result)
{
	throw Fall::Exception(message + " (error code " + std::to_string(result) + ")");
}

void lzmaCompress(const ByteBuffer & input, ByteBuffer & output, uint32_t compressionLevel)
{
	lzma_stream stream = LZMA_STREAM_INIT;
	auto endStream = [&]() { lzma_end(&stream); };
	try
	{
		auto result = lzma_easy_encoder(&stream, compressionLevel, LZMA_CHECK_NONE);
		if(result != LZMA_OK)
			throwLzma("Failed to initialize LZMA encoder", result);
		// Preemptively add 1 KiB to deal with compression overhead
		output.resize(input.size() + 1024);
		stream.next_in = input.data();
		stream.avail_in = input.size();
		stream.next_out = output.data();
		stream.avail_out = output.size();
		result = lzma_code(&stream, LZMA_FINISH);
		if(result != LZMA_STREAM_END)
			throwLzma("Failed to compress LZMA data", result);
		std::size_t compressedDataSize = output.size() - stream.avail_out;
		output.resize(compressedDataSize);
		endStream();
	}
	catch(...)
	{
		endStream();
		throw;
	}
}

void lzmaDecompress(const ByteBuffer & input, ByteBuffer & output)
{
	lzma_stream stream = LZMA_STREAM_INIT;
	auto endStream = [&]() { lzma_end(&stream); };
	try
	{
		auto result = lzma_stream_decoder(&stream, UINT64_MAX, LZMA_CONCATENATED);
		if(result != LZMA_OK)
			throwLzma("Failed to initialize LZMA decoder", result);
		// 1 MiB should suffice to decompress everything in one go
		const std::size_t bufferSize = 1024 * 1024;
		output.resize(bufferSize);
		stream.next_in = input.data();
		stream.avail_in = input.size();
		stream.next_out = output.data();
		stream.avail_out = output.size();
		result = lzma_code(&stream, LZMA_FINISH);
		if(result != LZMA_STREAM_END)
			throwLzma("Failed to decompress LZMA data", result);
		std::size_t decompressedDataSize = output.size() - stream.avail_out;
		output.resize(decompressedDataSize);
		endStream();
	}
	catch(...)
	{
		endStream();
		throw;
	}
}