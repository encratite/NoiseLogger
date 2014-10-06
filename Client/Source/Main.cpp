#include <iostream>

#include <Fall/Configuration.hpp>
#include <Fall/Exception.hpp>

namespace
{
	const std::string configurationPath("NoiseLogger.conf");
}

void runNoiseLogger()
{
	Fall::Configuration configuration(configurationPath);
	std::string deviceName = configuration.getString("deviceName", "default");
	// PCM sample rate, in Hz
	unsigned sampleRate = configuration.getNumber<unsigned>("sampleRate", 8000);
	// ALSA PCM latency, in milliseconds
	unsigned latency = configuration.getNumber<unsigned>("latency", 100);
	// Logging interval length, in milliseconds
	unsigned loggingInterval = configuration.getNumber<unsigned>("loggingInterval", 1000);
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