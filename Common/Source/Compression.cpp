#include <lzma.h>

#include <Common/Compression.hpp>

void lzmaCompress(const ByteBuffer & input, ByteBuffer & output)
{
	lzma_stream stream = LZMA_STREAM_INIT;
	auto endStream = [&] () { lzma_end(&stream); };
	try
	{
		auto result = lzma_easy_encoder(&stream, _compressionLevel, LZMA_CHECK_NONE);
		if(result != LZMA_OK)
			throw Fall::Exception("lzma_easy_encoder failed (" + std::to_string(result) + ")");
		// Preemptively add 1 KiB to deal with compression overhead
		output.resize(input.size() + 1024);
		stream.next_in = input.data();
		stream.avail_in = input.size();
		stream.next_out = output.data();
		stream.avail_out = output.size();
		result = lzma_code(&stream, LZMA_FINISH);
		if(result != LZMA_STREAM_END)
			throw Fall::Exception("lzma_code failed (" + std::to_string(result) + ")");
		std::size_t compressedDataSize = compressedData.size() - stream.avail_out;
		output.resize(compressedDataSize);
		endStream();
	}
	catch(...)
	{
		endStream();
		throw;
	}
}