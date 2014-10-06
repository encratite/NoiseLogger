#include <iostream>

#include <Client/NoiseLogger.hpp>

NoiseLogger::NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, unsigned loggingInterval):
	_pcm(deviceName, SND_PCM_FORMAT_S16, sampleRate, latency, loggingInterval)
{
}

void NoiseLogger::run()
{
	_pcm.open();
	std::size_t frameCount;
	const SampleType * buffer = _pcm.getBuffer(frameCount);
	_pcm.read();
	for(std::size_t i = 0; i < 100 && i < frameCount; i++)
	{
		SampleType sample = buffer[i];
		std::cout << sample << " ";
	}
	std::cout << std::endl;
}