#include <cmath>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <chrono>

#include <endian.h>
#include <lzma.h>

#include <Client/NoiseLogger.hpp>

namespace
{
	const std::size_t serializedValueSize = sizeof(uint64_t) + sizeof(float);
}

NoiseLogger::NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, unsigned loggingInterval, std::size_t valuesPerPacket, uint32_t compressionLevel):
	_pcm(deviceName, SND_PCM_FORMAT_S16, sampleRate, latency, loggingInterval),
	_valuesPerPacket(valuesPerPacket),
	_compressionLevel(compressionLevel)
{
}

void NoiseLogger::run()
{
	_pcm.open();
	while(true)
		readSamples();
}

void NoiseLogger::readSamples()
{
	_pcm.read();
	const auto & buffer = _pcm.getBuffer();
	uint64_t sum = 0;
	for(auto sample : buffer)
		sum += sample * sample;
	float rootMeanSquare = std::sqrt(static_cast<float>(sum / buffer.size()));
	auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
	uint64_t unixMilliseconds = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
	uint64_t reorderedMilliseconds = htobe64(unixMilliseconds);
	uint8_t const * millisecondsPointer = reinterpret_cast<uint8_t *>(&reorderedMilliseconds);
	for(std::size_t i = 0; i < sizeof(uint64_t); i++)
	{
		uint8_t byte = millisecondsPointer[i];
		_serializedData.push_back(byte);
	}
	uint8_t const * rootMeanSquarePointer = reinterpret_cast<uint8_t *>(&rootMeanSquare);
	for(std::size_t i = 0; i < sizeof(float); i++)
	{
		uint8_t byte = rootMeanSquarePointer[i];
		_serializedData.push_back(byte);
	}
	std::cout << unixMilliseconds << ": " << rootMeanSquare << " (" << _serializedData.size() << ")" << std::endl;
	if(_serializedData.size() / serializedValueSize >= _valuesPerPacket)
	{
		try
		{
			sendPacket();
		}
		catch(Fall::Exception const & exception)
		{
			std::cerr << "Failed to tansmit packet: " << exception.getMessage() << std::endl;
		}
		_serializedData.clear();
	}
}

void NoiseLogger::sendPacket()
{
	lzma_stream stream = LZMA_STREAM_INIT;
	try
	{
		auto result = lzma_easy_encoder(&stream, _compressionLevel, LZMA_CHECK_NONE);
		if(result != LZMA_OK)
			throw Fall::Exception("lzma_easy_encoder failed (" + std::to_string(result) + ")");
		std::vector<uint8_t> compressedData(_serializedData.size());
		stream.next_in = _serializedData.data();
		stream.avail_in = _serializedData.size();
		stream.next_out = compressedData.data();
		stream.avail_out = compressedData.size();
		result = lzma_code(&stream, LZMA_FINISH);
		if(result != LZMA_STREAM_END)
			throw Fall::Exception("lzma_code failed (" + std::to_string(result) + ")");
		std::size_t compressedDataSize = compressedData.size() - stream.avail_out;
		compressedData.resize(compressedDataSize);
		lzma_end(&stream);
		double compressionRatio = (compressedData.size() * 100.0) / _serializedData.size();
		std::cout << "Compression ratio: " << std::fixed << std::setprecision(1) << compressionRatio << "%" << std::endl;
	}
	catch(...)
	{
		lzma_end(&stream);
		throw;
	}
}