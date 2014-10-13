#pragma once

#include <cstdint>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>

#include <Client/AlsaPcm.hpp>
#include <Client/LogSample.hpp>

typedef int16_t SampleType;
typedef std::vector<LogSample> LogSamples;
typedef std::queue<LogSamples> PacketQueue;
typedef std::lock_guard<std::mutex> Lock;

enum NoiseLoggerState
{
	NoiseLoggerStateIdle,
	NoiseLoggerStateRunning,
	NoiseLoggerStateTerminating
};

class NoiseLogger
{
public:
	NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, uint16_t readInterval, std::size_t samplesPerPacket, uint32_t compressionLevel);
	~NoiseLogger();
	
	void run();
	
private:
	NoiseLoggerState _state;
	AlsaPcm<SampleType> _pcm;
	LogSamples _logSamples;
	uint16_t _readInterval;
	std::size_t _samplesPerPacket;
	uint32_t _compressionLevel;
	std::thread _communicationThread;
	std::mutex _packetQueueMutex;
	PacketQueue _packetQueue;
	
	void readSamples();
	void communicate();
	void sendPacket(const LogSamples & logSamples);
};