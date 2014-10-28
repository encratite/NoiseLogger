#include <cstring>

#include <Fall/Console.hpp>

#include <Common/Compression.hpp>
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
		if(_buffer.size() < lengthPrefixSize)
		{
			read();
			continue;
		}
		std::size_t offset = 0;
		uint16_t lengthPrefix = deserializeUInt16(_buffer, offset);
		std::size_t packetSize = lengthPrefixSize + lengthPrefix;
		if(_buffer.size() < packetSize)
		{
			read();
			continue;
		}
		ByteBuffer compressedPacket(packetSize - lengthPrefixSize);
		std::memcpy(compressedPacket.data(), _buffer.data() + lengthPrefixSize, compressedPacket.size());
		ByteBuffer newBuffer(_buffer.size() - packetSize);
		std::memcpy(newBuffer.data(), _buffer.data() + packetSize, newBuffer.size());
		_buffer = newBuffer;
		ByteBuffer decompressedPacket;
		lzmaDecompress(compressedPacket, decompressedPacket);
		logPacket.deserialize(decompressedPacket);
		break;
	}
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
	_client->read(_buffer);
}