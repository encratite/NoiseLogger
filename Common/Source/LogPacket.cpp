#include <Common/LogPacket.hpp>
#include <Common/Serialization.hpp>

LogPacket::LogPacket(uint64_t initialTimestamp, uint16_t interval, const std::vector<uint16_t> & samples) :
	initialTimestamp(initialTimestamp),
	interval(interval),
	samples(samples)
{
}

LogPacket::LogPacket(const ByteBuffer & buffer)
{
	deserialize(buffer);
}

void LogPacket::serialize(ByteBuffer & buffer)
{
	serializeUInt64(initialTimestamp, buffer);
	serializeUInt16(interval, buffer);
	for(uint16_t sample : samples)
		serializeUInt16(sample, buffer);
}

void LogPacket::deserialize(const ByteBuffer & buffer)
{
	std::size_t offset = 0;
	initialTimestamp = deserializeUInt64(buffer, offset);
	interval = deserializeUInt16(buffer, offset);
	while(offset < buffer.size())
	{
		uint16_t sample = deserializeUInt16(buffer, offset);
		samples.push_back(sample);
	}
}