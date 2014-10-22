#pragma once

#include <Common/SslSocket.hpp>

class SslClient: public SslSocket
{
public:
	void connect(const std::string & host, uint16_t port, const std::string & certificatePath);
	
	std::size_t read(void * buffer, std::size_t bufferSize);
	void read(ByteBuffer & buffer);
	
	void write(const void * buffer, std::size_t size);
	void write(const ByteBuffer & buffer);
	
	void setClientSocket(int socket, const std::string & certificatePath);
};