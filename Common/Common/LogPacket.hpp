#pragma once

#include <cstdint>
#include <vector>

#include <Common/Types.hpp>

struct LogPacket
{
	// UNIX milliseconds UTC timestamp
	uint64_t initialTimestamp;
	// Number of milliseconds between successive samples
	uint16_t interval;
	// Root mean square samples
	std::vector<uint16_t> samples;
	
	LogPacket(uint64_t initialTimestamp, uint16_t interval, const std::vector<uint16_t> & samples);
	LogPacket(const ByteBuffer & buffer);
	
	void serialize(ByteBuffer & buffer);
	void deserialize(const ByteBuffer & buffer);
};