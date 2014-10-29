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
	ClientConfiguration clientConfiguration;
	clientConfiguration.deviceName = configuration.getString("deviceName", "default");
	clientConfiguration.sampleRate = configuration.getNumber<unsigned>("sampleRate", 11025);
	clientConfiguration.latency = configuration.getNumber<unsigned>("latency", 100);
	clientConfiguration.readInterval = configuration.getNumber<unsigned>("readInterval", 100);
	clientConfiguration.samplesPerPacket = configuration.getNumber<std::size_t>("samplesPerPacket", 600);
	clientConfiguration.maximumPacketQueueSize = configuration.getNumber<std::size_t>("maximumPacketQueueSize", 10);
	clientConfiguration.compressionLevel = configuration.getNumber<uint32_t>("compressionLevel", 6);
	clientConfiguration.serverHost = configuration.getString("serverHost");
	clientConfiguration.serverPort = configuration.getNumber<uint16_t>("serverPort");
	clientConfiguration.certificatePath = configuration.getString("certificatePath");
	clientConfiguration.certificateAuthorityPath = configuration.getString("certificateAuthorityPath");
	clientConfiguration.reconnectDelay = configuration.getNumber<unsigned>("reconnectDelay", 10);
	NoiseLoggerClient logger(clientConfiguration);
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