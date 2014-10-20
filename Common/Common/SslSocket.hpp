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
	
	void connect(const std::string & host, uint16_t port, const std::string & certificatePath);
	void bindAndListen(uint16_t port, const std::string & certificatePath);
	void close();
	
	std::size_t read(void * buffer, std::size_t bufferSize);
	void read(ByteBuffer & buffer);
	
	void write(const void * buffer, std::size_t size);
	void write(const ByteBuffer & buffer);
	
private:
	int _socket;
	SSL_CTX * _sslContext;
	SSL * _ssl;
	
	void initializeSsl();
	void checkSocket();
	void createSslContext(const std::string & certificatePath);
	
	void closeAndThrow(const std::string & message);
	void closeAndThrowErrno(const std::string & message);
	void closeAndThrowSsl(const std::string & message);
};