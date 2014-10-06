#pragma once

#include <Client/AlsaPcm.hpp>

class NoiseLogger
{
public:
	typedef short SampleType;
	NoiseLogger(const std::string & deviceName, unsigned sampleRate, unsigned latency, unsigned loggingInterval);
	
	void run();
	
private:
	AlsaPcm<short> _pcm;
};