#include <cstring>

#include <Fall/Console.hpp>

#include <Common/Compression.hpp>
#include <Common/Debug.hpp>
#include <Server/NoiseLoggerServerClient.hpp>

NoiseLoggerServerClient::NoiseLoggerServerClient(SslClientPointer client):
	_client(client)
{
	_address = client->getAddress();
}
	
void NoiseLoggerServerClient::readPacket(LogPacket & logPacket)
{
	const std::size_t lengthPrefixSize = sizeof(uint16_t);
	while(true)
	{
		log("Buffer size: " + std::to_string(_buffer.size()));
		if(_buffer.size() < lengthPrefixSize)
		{
			read();
			continue;
		}
		std::size_t offset = 0;
		uint16_t lengthPrefix = deserializeUInt16(_buffer, offset);
		log("Length prefix: " + std::to_string(lengthPrefix));
		std::size_t packetSize = lengthPrefixSize + lengthPrefix;
		log("Packet size: " + std::to_string(packetSize));
		if(_buffer.size() < packetSize)
		{
			read();
			continue;
		}
		DEBUG_MARK
		ByteBuffer compressedPacket(packetSize - lengthPrefixSize);
		std::memcpy(compressedPacket.data(), _buffer.data() + lengthPrefixSize, compressedPacket.size());
		DEBUG_MARK
		ByteBuffer newBuffer(_buffer.size() - packetSize);
		std::memcpy(newBuffer.data(), _buffer.data() + packetSize, newBuffer.size());
		_buffer = newBuffer;
		DEBUG_MARK
		ByteBuffer decompressedPacket;
		lzmaDecompress(compressedPacket, decompressedPacket);
		DEBUG_MARK
		log("Decompressed size: " + decompressedPacket.size());
		logPacket.deserialize(decompressedPacket);
		DEBUG_MARK
		break;
	}
	DEBUG_MARK
}

void NoiseLoggerServerClient::log(const std::string & text)
{
	Fall::log("[" + _address + "] " + text);
}

const std::string & NoiseLoggerServerClient::getAddress() const
{
	return _address;
}

void NoiseLoggerServerClient::read()
{
	DEBUG_MARK
	_client->read(_buffer);
	DEBUG_MARK
}