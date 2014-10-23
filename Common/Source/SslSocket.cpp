#include <mutex>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <openssl/err.h>

#include <Common/SslSocket.hpp>

namespace
{
	std::mutex sslInitializationMutex;
	bool sslHasBeenInitialized = false;
	
	const int invalidSocket = -1;
}

SslSocket::~SslSocket()
{
	close();
}

SslSocket::SslSocket():
	_socket(invalidSocket),
	_sslContext(nullptr),
	_ownsContext(false)
{
	initializeSsl();
}

void SslSocket::close()
{
	if(_ssl != nullptr)
	{
		SSL_free(_ssl);
		_ssl = nullptr;
	}
	if(_ownsContext && _sslContext != nullptr)
	{
		SSL_CTX_free(_sslContext);
		_sslContext = nullptr;
	}
	if(!isInvalidSocket())
	{
		::close(_socket);
		_socket = invalidSocket;
	}
}

void SslSocket::initializeSsl()
{
	std::lock_guard<std::mutex> lock(sslInitializationMutex);
	if(!sslHasBeenInitialized)
	{
		SSL_library_init();
		SSL_load_error_strings();
		sslHasBeenInitialized = true;
	}
}

void SslSocket::createSocket(const addrinfo & addressInfo)
{
	_socket = socket(addressInfo.ai_family, addressInfo.ai_socktype, addressInfo.ai_protocol);
	if(isInvalidSocket())
		closeAndThrowErrno("Failed to create socket");
}

void SslSocket::createSslContext(bool isClient, const std::string & certificatePath)
{
	auto method = isClient ? TLSv1_1_client_method() : TLSv1_1_server_method();
	_sslContext = SSL_CTX_new(method);
	if(_sslContext == nullptr)
		closeAndThrow("Failed to create SSL context");
	_ownsContext = true;
	SSL_CTX_set_verify(_sslContext, SSL_VERIFY_PEER, nullptr);
	int result = SSL_CTX_use_certificate_chain_file(_sslContext, certificatePath.c_str());
	if(result != 1)
		closeAndThrowSsl("Unable to load certificate file");
	result = SSL_CTX_use_PrivateKey_file(_sslContext, certificatePath.c_str(), SSL_FILETYPE_PEM);
	if(result != 1)
		closeAndThrowSsl("Unable to load private key file");
	result = SSL_CTX_check_private_key(_sslContext);
	if(result != 1)
		closeAndThrowSsl("Private key does not match certificate");
	createSslStructure();
}

void SslSocket::createSslStructure()
{
	_ssl = SSL_new(_sslContext);
	if(_ssl == nullptr)
		closeAndThrow("Failed to create SSL structure");
	int result = SSL_set_fd(_ssl, _socket);
	if(result != 1)
		closeAndThrow("Failed to set SSL file descriptor");
}

void SslSocket::checkSocket()
{
	if(isInvalidSocket())
		throw Fall::Exception("Socket has not been initialized");
}

bool SslSocket::isInvalidSocket()
{
	return _socket == invalidSocket;
}

void SslSocket::closeAndThrow(const std::string & message)
{
	close();
	throw Fall::Exception(message);
}

void SslSocket::closeAndThrowErrno(const std::string & message)
{
	std::string exceptionMessage = (message + ": ") + strerror(errno);
	close();
	throw Fall::Exception(exceptionMessage);
}

void SslSocket::closeAndThrowSsl(const std::string & message)
{
	long error = ERR_get_error();
	char buffer[256];
	ERR_error_string_n(error, buffer, sizeof(buffer));
	std::string exceptionMessage = (message + ": ") + buffer;
	close();
	throw Fall::Exception(exceptionMessage);
}