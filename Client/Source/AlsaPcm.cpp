#include <Client/AlsaPcm.hpp>

template <typename SampleType>
	AlsaPcm<SampleType>::AlsaPcm(const std::string & deviceName, snd_pcm_format_t format, unsigned sampleRate, unsigned latency, unsigned bufferLength):
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

template <typename SampleType>
	AlsaPcm<SampleType>::~AlsaPcm()
{
	close();
}

template <typename SampleType>
	void AlsaPcm<SampleType>::open()
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

template <typename SampleType>
	void AlsaPcm<SampleType>::close()
{
	if(_buffer != nullptr)
		delete[] _buffer;
	if(_pcm != nullptr)
		snd_pcm_close(_pcm);
}

template <typename SampleType>
	void AlsaPcm<SampleType>::read()
{
	int error = snd_pcm_readi(_pcm, _buffer, static_cast<snd_pcm_uframes_t>(_frameCount));
	errorCheck("snd_pcm_readi", error);
}

template <typename SampleType>
	const SampleType * AlsaPcm<SampleType>::getBuffer(std::size_t & frameCount)
{
	frameCount = _frameCount;
	return _buffer;
}

template <typename SampleType>
	void AlsaPcm<SampleType>::errorCheck(const std::string & functionName, int error)
{
	if(error < 0)
		throw AlsaException(functionName, error);
}