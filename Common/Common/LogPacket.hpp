#pragma once

#include <cstdint>
#include <vector>

struct LogPacket
{
	// UNIX milliseconds UTC timestamp
	uint64_t initialTimestamp;
	// Number of milliseconds between successive values
	uint16_t interval;
	// Root mean square values
	std::vector<uint16_t> values;
	
	LogPacket(uint64_t initialTimestamp, uint16_t interval, std::vector<uint16_t> const & values);
	
	void serialize(std::vector<uint8_t> & buffer);
};