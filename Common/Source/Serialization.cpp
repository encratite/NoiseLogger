#include <endian.h>

#include <Common/Serialization.hpp>

void serializeUInt16(uint16_t integer, ByteBuffer & buffer)
{
	uint16_t serializableInteger = htobe16(integer);
	serialize<uint16_t>(serializableInteger, buffer);
}

void serializeUInt32(uint32_t integer, ByteBuffer & buffer)
{
	uint32_t serializableInteger = htobe32(integer);
	serialize<uint32_t>(serializableInteger, buffer);
}

void serializeUInt64(uint64_t integer, ByteBuffer & buffer)
{
	uint64_t serializableInteger = htobe64(integer);
	serialize<uint64_t>(serializableInteger, buffer);
}

uint16_t deserializeUInt16(const ByteBuffer & buffer, std::size_t & offset)
{
	uint16_t bigEndianInteger = deserialize<uint16_t>(buffer, offset);
	uint16_t output = be16toh(bigEndianInteger);
	offset += sizeof(uint16_t);
	return output;
}

uint32_t deserializeUInt32(const ByteBuffer & buffer, std::size_t & offset)
{
	uint32_t bigEndianInteger = deserialize<uint32_t>(buffer, offset);
	uint32_t output = be32toh(bigEndianInteger);
	offset += sizeof(uint32_t);
	return output;
}

uint64_t deserializeUInt64(const ByteBuffer & buffer, std::size_t & offset)
{
	uint64_t bigEndianInteger = deserialize<uint64_t>(buffer, offset);
	uint64_t output = be64toh(bigEndianInteger);
	offset += sizeof(uint64_t);
	return output;
}