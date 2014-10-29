#pragma once

#include <cstdint>
#include <string>

#include <openssl/ssl.h>

#include <Fall/Exception.hpp>

#include <Common/AddressInfo.hpp>
#include <Common/Types.hpp>

class SslSocket
{
public:
	~SslSocket();
	
protected:
	int _socket;
	SSL_CTX * _sslContext;
	SSL * _ssl;
	bool _ownsContext;
	
	SslSocket();
	
	void close();
	
	void initializeSsl();
	void createSocket(const addrinfo & addressInfo);
	void createSslContext(bool isClient, const std::string & certificatePath, const std::string & certificateAuthorityPath);
	void createSslStructure();
	
	void checkSocket();
	bool isInvalidSocket();
	
	void closeAndThrow(const std::string & message);
	void closeAndThrowErrno(const std::string & message);
	void closeAndThrowSsl(const std::string & message);
};