#pragma once

#include <string>

#include <alsa/asoundlib.h>

#include <Client/AlsaException.hpp>

template <typename SampleType>
	class AlsaPcm
{
public:
	// Device name is a string such as "default" or "HW:1,0" where 1 is the device and 0 the sub-device
	// Sample rate in Hz
	// Latency in milliseconds
	// Buffer length in milliseconds
	AlsaPcm(const std::string & deviceName, snd_pcm_format_t format, unsigned sampleRate, unsigned latency, unsigned bufferLength);
	~AlsaPcm();
	
	void open();
	void close();
	
	const SampleType * read();
	
private:
	std::string _deviceName;
	snd_pcm_format_t _format;
	unsigned _sampleRate;
	unsigned _latency;
	unsigned _bufferLength;
	std::size_t _sampleCount;
	
	snd_pcm_t * _pcm;
	SampleType * _buffer;
	
	void errorCheck(std::string const & functionName, int error);
};