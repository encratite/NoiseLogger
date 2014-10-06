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
	AlsaPcm(const std::string & deviceName, snd_pcm_format_t format, unsigned sampleRate, unsigned latency, unsigned bufferLength):
		_deviceName(deviceName),
		_format(format),
		_sampleRate(sampleRate),
		_latency(latency),
		_bufferLength(bufferLength),
		_pcm(nullptr),
		_buffer(nullptr)
	{
		_frameCount = static_cast<std::size_t>((sampleRate * bufferLength) / 1000);
	}

	~AlsaPcm()
	{
		close();
	}

	void open()
	{
		if(_pcm != nullptr)
			throw new Fall::Exception("ALSA PCM has already been opened");
		// Use blocking read/write mode
		int openMode = 0;
		int error = snd_pcm_open(&_pcm, _deviceName.c_str(), SND_PCM_STREAM_CAPTURE, openMode);
		errorCheck("snd_pcm_open", error);
		// Mono recording
		unsigned channelCount = 1;
		// Disable software resampling
		int enableSoftwareResampling = 0;
		unsigned latencyMicroseconds = _latency * 1000;
		error = snd_pcm_set_params(_pcm, _format, SND_PCM_ACCESS_RW_INTERLEAVED, channelCount, _sampleRate, enableSoftwareResampling, latencyMicroseconds);
		errorCheck("snd_pcm_set_params", error);
		_buffer = new SampleType[_frameCount];
	}

	void close()
	{
		if(_buffer != nullptr)
			delete[] _buffer;
		if(_pcm != nullptr)
			snd_pcm_close(_pcm);
	}

	void read()
	{
		int error = snd_pcm_readi(_pcm, _buffer, static_cast<snd_pcm_uframes_t>(_frameCount));
		errorCheck("snd_pcm_readi", error);
	}

	const SampleType * getBuffer(std::size_t & frameCount)
	{
		frameCount = _frameCount;
		return _buffer;
	}
	
private:
	std::string _deviceName;
	snd_pcm_format_t _format;
	unsigned _sampleRate;
	unsigned _latency;
	unsigned _bufferLength;
	std::size_t _frameCount;
	
	snd_pcm_t * _pcm;
	SampleType * _buffer;
	
	void errorCheck(const std::string & functionName, int error)
	{
		if(error < 0)
			throw AlsaException(functionName, error);
	}
};