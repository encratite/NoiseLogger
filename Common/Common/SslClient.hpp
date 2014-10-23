#pragma once

#include <memory>

#include <Common/SslSocket.hpp>

class SslClient;

typedef std::shared_ptr<SslClient> SslClientPointer;

class SslClient: public SslSocket
{
public:
	void connect(const std::string & host, uint16_t port, const std::string & certificatePath);
	
	std::size_t read(void * buffer, std::size_t bufferSize);
	void read(ByteBuffer & buffer);
	
	void write(const void * buffer, std::size_t size);
	void write(const ByteBuffer & buffer);
	
	void setClientData(int socket, SSL_CTX * sslContext);
};