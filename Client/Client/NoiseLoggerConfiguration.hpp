#pragma once

#include <cstdint>
#include <string>

struct NoiseLoggerConfiguration
{
	// An ALSA device string such as "hw:1,0"
	std::string deviceName;
	// PCM sample rate, in Hz
	unsigned sampleRate;
	// ALSA PCM latency, in milliseconds
	unsigned latency;
	// Read interval length, in milliseconds
	uint16_t readInterval;
	// The number of root mean square samples to accumulate prior to compression and transmission
	std::size_t samplesPerPacket;
	// The maximum size of the packet queue, to prevent excessive memory use
	// If exceeded, the logger will start dropping sets of root mean square samples because compression and transmission are taking too long
	std::size_t maximumPacketQueueSize;
	// The LZMA compression level (0-9)
	uint32_t compressionLevel;
	// The host of the log server
	std::string logServerHost;
	// The TCP port of the log server
	uint16_t logServerPort;
	// The path to the X.509 client certificate to be used for TLS authentication with the log server
	std::string clientCertificatePath;
};