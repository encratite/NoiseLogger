#include <cmath>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <chrono>

#include <Client/NoiseLogger.hpp>
#include <Client/NoiseLoggerConfiguration.hpp>
#include <Common/Compression.hpp>
#include <Common/Debug.hpp>
#include <Common/LogPacket.hpp>
#include <Common/Serialization.hpp>

NoiseLogger::NoiseLogger(const NoiseLoggerConfiguration & configuration):
	_configuration(configuration),
	_state(NoiseLoggerStateIdle),
	_pcm(configuration.deviceName, SND_PCM_FORMAT_S16, configuration.sampleRate, configuration.latency, configuration.readInterval)
{
}

NoiseLogger::~NoiseLogger()
{
	_state = NoiseLoggerStateTerminating;
	_communicationThread.join();
}

void NoiseLogger::run()
{
	if(_state != NoiseLoggerStateIdle)
		throw Fall::Exception("Unable to start logger");
	_state = NoiseLoggerStateRunning;
	_pcm.open();
	_communicationThread = std::thread(&NoiseLogger::communicate, this);
	while(_state == NoiseLoggerStateRunning)
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
	if(_logSamples.size() >= _configuration.samplesPerPacket)
	{
		pushPacket();
		_logSamples.clear();
	}
}

void NoiseLogger::pushPacket()
{
	Lock lock(_packetQueueMutex);
	_packetQueue.push(_logSamples);
	while(_packetQueue.size() > _configuration.maximumPacketQueueSize)
		_packetQueue.pop();
	_packetAvailable.notify_one();
}

void NoiseLogger::popPacket(LogSamples & logSamples)
{
	Lock lock(_packetQueueMutex);
	if(_packetQueue.empty())
		_packetAvailable.wait(lock);
	if(_state != NoiseLoggerStateRunning)
		return;
	logSamples = _packetQueue.front();
	_packetQueue.pop();
}

void NoiseLogger::communicate()
{
	while(_state == NoiseLoggerStateRunning)
	{
		LogSamples logSamples;
		popPacket(logSamples);
		if(logSamples.empty())
			continue;
		sendPacket(logSamples);
	}
}

void NoiseLogger::sendPacket(const LogSamples & logSamples)
{
	const LogSample & firstSample = logSamples.front();
	std::vector<uint16_t> samples;
	for (const auto & logSample : logSamples)
		samples.push_back(logSample.sample);
	LogPacket packet(firstSample.timestamp, _configuration.readInterval, samples);
	ByteBuffer serializedData;
	packet.serialize(serializedData);
	ByteBuffer compressedData;
	lzmaCompress(serializedData, compressedData, _configuration.compressionLevel);
}