#pragma once

#include <cstdint>
#include <string>

#include <openssl/ssl.h>

#include <Fall/Exception.hpp>

#include <Common/Types.hpp>

class SslSocket
{
public:
	SslSocket();
	~SslSocket();
	
	void connect(const std::string & host, uint16_t port);
	void disconnect();
	
	std::size_t read(void * buffer, std::size_t bufferSize);
	void read(ByteBuffer & buffer);
	
	void write(const void * buffer, std::size_t size);
	void write(const ByteBuffer & buffer);
	
private:
	int _socket;
	SSL_CTX * _sslContext;
	
	void initializeSsl();
	void throwErrnoException(const std::string & message);
};