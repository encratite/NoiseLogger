#include <iostream>

#include <Client/NoiseLogger.hpp>

NoiseLogger::NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, unsigned loggingInterval):
	_pcm(deviceName, SND_PCM_FORMAT_FLOAT, sampleRate, latency, loggingInterval)
{
}

void NoiseLogger::run()
{
	_pcm.open();
	std::size_t frameCount;
	const float * frameBuffer = _pcm.getBuffer(frameCount);
	_pcm.read();
	std::cout << "Sample: " << frameBuffer[0] << std::endl;
}