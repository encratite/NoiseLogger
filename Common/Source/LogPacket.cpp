#include <Common/LogPacket.hpp>
#include <Common/Serialization.hpp>

LogPacket::LogPacket(uint64_t initialTimestamp, uint16_t interval, const std::vector<uint16_t> & samples) :
	initialTimestamp(initialTimestamp),
	interval(interval),
	samples(samples)
{
}

void LogPacket::serialize(std::vector<uint8_t> & buffer)
{
	serializeUInt64(initialTimestamp, buffer);
	serializeUInt16(interval, buffer);
	for(uint16_t sample : samples)
		serializeUInt16(sample, buffer);
}