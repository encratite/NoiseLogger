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