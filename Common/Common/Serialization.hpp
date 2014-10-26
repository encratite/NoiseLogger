#pragma once

#include <cstdint>

#include <Common/Types.hpp>

template <typename Type>
	void serialize(Type data, ByteBuffer & buffer)
{
	const uint8_t * pointer = reinterpret_cast<uint8_t *>(&data);
	for(std::size_t i = 0; i < sizeof(Type); i++)
	{
		uint8_t byte = pointer[i];
		buffer.push_back(byte);
	}
}

template <typename Type>
	Type deserialize(const void * buffer)
{
	Type output = *reinterpret_cast<const Type *>(buffer);
	return output;
}

template <typename Type>
	Type deserialize(const ByteBuffer & buffer, std::size_t offset)
{
	return deserialize<Type>(buffer.data() + offset);
}
	
void serializeUInt16(uint16_t integer, ByteBuffer & buffer);
void serializeUInt32(uint32_t integer, ByteBuffer & buffer);
void serializeUInt64(uint64_t integer, ByteBuffer & buffer);

uint16_t deserializeUInt16(const ByteBuffer & buffer, std::size_t & offset);
uint32_t deserializeUInt32(const ByteBuffer & buffer, std::size_t & offset);
uint64_t deserializeUInt64(const ByteBuffer & buffer, std::size_t & offset);