#pragma once

#include <Client/AlsaPcm.hpp>

class NoiseLogger
{
public:
	NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, unsigned loggingInterval);
	
	void run();
	
private:
	AlsaPcm<float> _pcm;
};