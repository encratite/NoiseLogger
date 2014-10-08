#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <chrono>

#include <Client/NoiseLogger.hpp>
#include <Common/Compression.hpp>
#include <Common/Debug.hpp>
#include <Common/LogPacket.hpp>
#include <Common/Serialization.hpp>

NoiseLogger::NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, uint16_t readInterval, std::size_t valuesPerPacket, uint32_t compressionLevel):
	_pcm(deviceName, SND_PCM_FORMAT_S16, sampleRate, latency, readInterval),
	_readInterval(readInterval),
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
	uint16_t rootMeanSquare = static_cast<uint16_t>(std::sqrt(static_cast<float>(sum) / buffer.size()));
	auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
	uint64_t unixMilliseconds = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
	LogSample logSample(unixMilliseconds, rootMeanSquare);
	_logSamples.push_back(logSample);
	std::cout << unixMilliseconds << ": " << rootMeanSquare << " (" << _logSamples.size() << " sample(s))" << std::endl;
	if(_logSamples.size() >= _valuesPerPacket)
	{
		try
		{
			sendPacket();
		}
		catch(Fall::Exception const & exception)
		{
			std::cerr << "Failed to tansmit packet: " << exception.getMessage() << std::endl;
		}
		_logSamples.clear();
	}
}

void NoiseLogger::sendPacket()
{
	DEBUG_MARK;
	LogSample const & firstSample = _logSamples.front();
	std::vector<uint16_t> values;
	for(auto const & logSample : _logSamples)
		values.push_back(logSample.value);
	DEBUG_MARK;
	LogPacket packet(firstSample.timestamp, _readInterval, values);
	DEBUG_MARK;
	ByteBuffer serializedData;
	DEBUG_MARK;
	packet.serialize(serializedData);
	DEBUG_MARK;
	ByteBuffer compressedData;
	DEBUG_MARK;
	lzmaCompress(serializedData, compressedData, _compressionLevel);
	DEBUG_MARK;
	double compressionRatio = (compressedData.size() * 100.0) / serializedData.size();
	std::cout << "Compression ratio: " << std::fixed << std::setprecision(1) << compressionRatio << "%" << std::endl;
	exit(0);
}