#pragma once

#include <cstdint>
#include <vector>

struct LogPacket
{
	// UNIX milliseconds UTC timestamp
	uint64_t initialTimestamp;
	// Number of milliseconds between successive samples
	uint16_t interval;
	// Root mean square samples
	std::vector<uint16_t> samples;
	
	LogPacket(uint64_t initialTimestamp, uint16_t interval, const std::vector<uint16_t> & samples);
	
	void serialize(std::vector<uint8_t> & buffer);
};