#pragma once

#include <cstdint>

struct LogSample
{
	uint64_t timestamp;
	uint16_t value;
	
	LogSample(uint64_t timestamp, uint16_t value);
};