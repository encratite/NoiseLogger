#include <Common/LogPacket.hpp>
#include <Common/Serialization.hpp>

LogPacket::LogPacket(uint64_t initialTimestamp, uint16_t interval, std::vector<uint16_t> const & values):
	initialTimestamp(initialTimestamp),
	interval(interval),
	values(values)
{
}

void LogPacket::serialize(std::vector<uint8_t> & buffer)
{
	serializeUInt64(initialTimestamp, buffer);
	serializeUInt16(interval, buffer);
	for(uint16_t value : values)
		serializeUInt16(value, buffer);
}