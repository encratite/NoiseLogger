#pragma once

#include <string>
#include <vector>

#include <alsa/asoundlib.h>

#include <Client/AlsaException.hpp>

template <typename SampleType>
	class AlsaPcm
{
public:
	// The device name is a string such as "default" or "hw:1,0" where 1 is the device and 0 the sub-device
	// Sample rate in Hz
	// Latency in milliseconds
	// Buffer length in milliseconds
	AlsaPcm(const std::string & deviceName, snd_pcm_format_t format, unsigned sampleRate, unsigned latency, unsigned bufferLength):
		_deviceName(deviceName),
		_format(format),
		_sampleRate(sampleRate),
		_latency(latency),
		_pcm(nullptr)
	{
		std::size_t frameCount = static_cast<std::size_t>((sampleRate * bufferLength) / 1000);
		_buffer.resize(frameCount);
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
	}

	void close()
	{
		if(_pcm != nullptr)
			snd_pcm_close(_pcm);
	}

	void read()
	{
		int result = snd_pcm_readi(_pcm, _buffer.data(), static_cast<snd_pcm_uframes_t>(_buffer.size()));
		if(result == -EPIPE)
		{
			snd_pcm_status_t * status;
			snd_pcm_status_alloca(&status);
			result = snd_pcm_status(_pcm, status);
			errorCheck("snd_pcm_status", result);
			result = snd_pcm_prepare(_pcm);
			errorCheck("snd_pcm_prepare", result);
		}
		else
		{
			errorCheck("snd_pcm_readi", result);
		}
	}

	const std::vector<SampleType> & getBuffer()
	{
		return _buffer;
	}
	
private:
	std::string _deviceName;
	snd_pcm_format_t _format;
	unsigned _sampleRate;
	unsigned _latency;
	
	snd_pcm_t * _pcm;
	std::vector<SampleType> _buffer;
	
	void errorCheck(const std::string & functionName, int error)
	{
		if(error < 0)
			throw AlsaException(functionName, error);
	}
};