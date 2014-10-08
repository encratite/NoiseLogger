#pragma once

#include <cstdint>
#include <vector>

#include <Client/AlsaPcm.hpp>
#include <Client/LogSample.hpp>

class NoiseLogger
{
public:
	typedef int16_t SampleType;
	
	NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, uint16_t readInterval, std::size_t valuesPerPacket, uint32_t compressionLevel);
	
	void run();
	
private:
	AlsaPcm<SampleType> _pcm;
	std::vector<LogSample> _logSamples;
	uint16_t _readInterval;
	std::size_t _valuesPerPacket;
	uint32_t _compressionLevel;
	
	void readSamples();
	void sendPacket();
};