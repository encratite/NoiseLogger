#include <cmath>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <chrono>

#include <Fall/Console.hpp>

#include <Client/NoiseLoggerClient.hpp>
#include <Client/ClientConfiguration.hpp>
#include <Common/Compression.hpp>
#include <Common/Debug.hpp>
#include <Common/LogPacket.hpp>
#include <Common/Serialization.hpp>

NoiseLoggerClient::NoiseLoggerClient(const ClientConfiguration & configuration):
	_configuration(configuration),
	_state(NoiseLoggerStateIdle),
	_pcm(configuration.deviceName, SND_PCM_FORMAT_S16, configuration.sampleRate, configuration.latency, configuration.readInterval)
{
}

NoiseLoggerClient::~NoiseLoggerClient()
{
	_state = NoiseLoggerStateTerminating;
	_communicationThread.join();
}

void NoiseLoggerClient::run()
{
	if(_state != NoiseLoggerStateIdle)
		throw Fall::Exception("Unable to start logger");
	_state = NoiseLoggerStateRunning;
	_pcm.open();
	_communicationThread = std::thread(&NoiseLoggerClient::communicate, this);
	while(_state == NoiseLoggerStateRunning)
		readSamples();
}

void NoiseLoggerClient::readSamples()
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
	// Fall::log(std::to_string(unixMilliseconds) + ": " + std::to_string(rootMeanSquare) + " (" + std::to_string(_logSamples.size()) + " sample(s))");
	if(_logSamples.size() >= _configuration.samplesPerPacket)
	{
		pushPacket();
		_logSamples.clear();
	}
}

void NoiseLoggerClient::pushPacket()
{
	Lock lock(_packetQueueMutex);
	_packetQueue.push(_logSamples);
	while(_packetQueue.size() > _configuration.maximumPacketQueueSize)
		_packetQueue.pop();
	_packetAvailable.notify_one();
}

void NoiseLoggerClient::popPacket(LogSamples & logSamples)
{
	Lock lock(_packetQueueMutex);
	if(_packetQueue.empty())
		_packetAvailable.wait(lock);
	if(_state != NoiseLoggerStateRunning)
		return;
	logSamples = _packetQueue.front();
	_packetQueue.pop();
}

void NoiseLoggerClient::communicate()
{
	while(_state == NoiseLoggerStateRunning)
	{
		bool connected = connect();
		if(!connected)
			continue;
		LogSamples logSamples;
		popPacket(logSamples);
		if(logSamples.empty())
			continue;
		sendPacket(logSamples);
	}
}

bool NoiseLoggerClient::connect()
{
	try
	{
		if(!_sslClient.isConnected())
		{
			Fall::log("Connecting to server");
			_sslClient.connect(_configuration.serverHost, _configuration.serverPort, _configuration.certificatePath, _configuration.certificateAuthorityPath);
			Fall::log("Connected to server");
		}
		return true;
	}
	catch(const Fall::Exception & exception)
	{
		Fall::log(exception.getMessage());
		if(_state == NoiseLoggerStateRunning)
		{
			unsigned delay = _configuration.reconnectDelay;
			Fall::log("Trying again in " + std::to_string(delay) + " second(s)");
			std::chrono::seconds duration(delay);
			std::this_thread::sleep_for(duration);
		}
		return false;
	}
}

void NoiseLoggerClient::sendPacket(const LogSamples & logSamples)
{
	Fall::log("Sending packet to server");
	const LogSample & firstSample = logSamples.front();
	std::vector<uint16_t> samples;
	for (const auto & logSample : logSamples)
		samples.push_back(logSample.sample);
	LogPacket packet(firstSample.timestamp, _configuration.readInterval, samples);
	ByteBuffer serializedData;
	packet.serialize(serializedData);
	ByteBuffer compressedData;
	lzmaCompress(serializedData, compressedData, _configuration.compressionLevel);
	ByteBuffer output;
	serializeUInt16(compressedData.size(), output);
	output.insert(output.end(), compressedData.begin(), compressedData.end());
	try
	{
		_sslClient.write(output);
	}
	catch(const Fall::Exception & exception)
	{
		Fall::log(exception.getMessage());
	}
}