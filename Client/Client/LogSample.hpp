#pragma once

#include <cstdint>

struct LogSample
{
	uint64_t timestamp;
	uint16_t sample;
	
	LogSample(uint64_t timestamp, uint16_t sample);
};