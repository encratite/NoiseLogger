#include <iostream>

#include <Fall/Configuration.hpp>
#include <Fall/Exception.hpp>

#include <Client/NoiseLoggerClient.hpp>
#include <Client/ClientConfiguration.hpp>

namespace
{
	const std::string configurationPath("NoiseLoggerClient.conf");
}

void runClient()
{
	Fall::Configuration configuration(configurationPath);
	ClientConfiguration loggerConfiguration;
	loggerConfiguration.deviceName = configuration.getString("deviceName", "default");
	loggerConfiguration.sampleRate = configuration.getNumber<unsigned>("sampleRate", 11025);
	loggerConfiguration.latency = configuration.getNumber<unsigned>("latency", 100);
	loggerConfiguration.readInterval = configuration.getNumber<unsigned>("readInterval", 100);
	loggerConfiguration.samplesPerPacket = configuration.getNumber<std::size_t>("samplesPerPacket", 600);
	loggerConfiguration.maximumPacketQueueSize = configuration.getNumber<std::size_t>("maximumPacketQueueSize", 10);
	loggerConfiguration.compressionLevel = configuration.getNumber<uint32_t>("compressionLevel", 6);
	loggerConfiguration.serverHost = configuration.getString("serverHost");
	loggerConfiguration.serverPort = configuration.getNumber<uint16_t>("serverPort");
	loggerConfiguration.certificatePath = configuration.getString("certificatePath");
	loggerConfiguration.reconnectDelay = configuration.getNumber<unsigned>("reconnectDelay", 10);
	NoiseLoggerClient logger(loggerConfiguration);
	logger.run();
}

int main(int argc, char ** argv)
{
	try
	{
		runClient();
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