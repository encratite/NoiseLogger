#pragma once

#include <string>

#include <Common/SslClient.hpp>
#include <Common/LogPacket.hpp>
#include <Common/Types.hpp>

class NoiseLoggerServerClient
{
public:
	NoiseLoggerServerClient(SslClientPointer client);
	
	void readPacket(LogPacket & logPacket);
	void log(const std::string & text);
	const std::string & getAddress() const;
	
private:
	SslClientPointer _client;
	std::string _address;
	ByteBuffer _buffer;
	
	void read();
};