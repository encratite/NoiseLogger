#pragma once

#include <cstdint>
#include <queue>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <Client/AlsaPcm.hpp>
#include <Client/LogSample.hpp>
#include <Client/NoiseLoggerConfiguration.hpp>
#include <Common/SslClient.hpp>

typedef int16_t SampleType;
typedef std::vector<LogSample> LogSamples;
typedef std::queue<LogSamples> PacketQueue;
typedef std::unique_lock<std::mutex> Lock;

enum NoiseLoggerState
{
	NoiseLoggerStateIdle,
	NoiseLoggerStateRunning,
	NoiseLoggerStateTerminating
};

class NoiseLogger
{
public:
	NoiseLogger(const NoiseLoggerConfiguration & configuration);
	~NoiseLogger();
	
	void run();
	
private:
	NoiseLoggerConfiguration _configuration;
	NoiseLoggerState _state;
	AlsaPcm<SampleType> _pcm;
	LogSamples _logSamples;
	std::thread _communicationThread;
	std::mutex _packetQueueMutex;
	std::condition_variable _packetAvailable;
	PacketQueue _packetQueue;
	SslClient _sslClient;
	
	void readSamples();
	void pushPacket();
	void popPacket(LogSamples & logSamples);
	void communicate();
	bool connect();
	void sendPacket(const LogSamples & logSamples);
};