#pragma once

#include <cstdint>
#include <vector>

#include <Client/AlsaPcm.hpp>

class NoiseLogger
{
public:
	typedef int16_t SampleType;
	
	NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, unsigned loggingInterval, std::size_t valuesPerPacket, uint32_t compressionLevel);
	
	void run();
	
private:
	AlsaPcm<SampleType> _pcm;
	std::vector<uint8_t> _serializedData;
	std::size_t _valuesPerPacket;
	uint32_t _compressionLevel;
	
	void readSamples();
	void sendPacket();
};