#include <iostream>

#include <Fall/Configuration.hpp>
#include <Fall/Exception.hpp>

#include <Client/NoiseLogger.hpp>

namespace
{
	const std::string configurationPath("NoiseLogger.conf");
}

void runNoiseLogger()
{
	Fall::Configuration configuration(configurationPath);
	// An ALSA device string such as "hw:1,0"
	std::string deviceName = configuration.getString("deviceName", "default");
	// PCM sample rate, in Hz
	unsigned sampleRate = configuration.getNumber<unsigned>("sampleRate", 11025);
	// ALSA PCM latency, in milliseconds
	unsigned latency = configuration.getNumber<unsigned>("latency", 100);
	// Read interval length, in milliseconds
	unsigned readInterval = configuration.getNumber<unsigned>("readInterval", 100);
	std::size_t samplesPerPacket = configuration.getNumber<std::size_t>("samplesPerPacket", 600);
	uint32_t compressionLevel = configuration.getNumber<uint32_t>("compressionLevel", 6);
	NoiseLogger logger(deviceName, sampleRate, latency, readInterval, samplesPerPacket, compressionLevel);
	logger.run();
}

int main(int argc, char * * argv)
{
	try
	{
		runNoiseLogger();
	}
	catch(const Fall::Exception & exception)
	{
		std::cerr << "Unhandled application exception: " << exception.getMessage() << std::endl;
		return 1;
	}
	catch(const std::exception & exception)
	{
		std::cerr << "Unhandled standard exception: " << exception.what() << std::endl;
		return 2;
	}
	catch(...)
	{
		std::cerr << "Unhandled exception of unknown type" << std::endl;
		return 3;
	}
	return 0;
}