#include <mutex>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <Common/SslSocket.hpp>

namespace
{
	std::mutex sslInitializationMutex;
	bool sslHasBeenInitialized = false;
	
	const int invalidSocket = -1;
	
	const std::size_t defaultBufferSize = 4 * 1024;
}

SslSocket::SslSocket():
	_socket(invalidSocket),
	_sslContext(nullptr)
{
	initializeSsl();
}

SslSocket::~SslSocket()
{
	disconnect();
}
	
void SslSocket::connect(const std::string & host, uint16_t port)
{
	if(_socket != invalidSocket)
		throw Fall::Exception("Unable to connect because the socket is already in use");
	disconnect();
	_socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(_socket == invalidSocket)
		throwErrnoException("Failed to create socket");
	throw Fall::Exception("Not implemented");
}

void SslSocket::disconnect()
{
	if(_sslContext != nullptr)
	{
		::SSL_CTX_free(_sslContext);
		_sslContext = nullptr;
	}
	if(_socket != invalidSocket)
	{
		::close(_socket);
		_socket = invalidSocket;
	}
}
	
std::size_t SslSocket::read(void * buffer, std::size_t bufferSize)
{
	std::size_t bytesRead = ::recv(_socket, buffer, bufferSize, 0);
	if(bytesRead < 0)
		throwErrnoException("Failed to read from socket");
	else if(bytesRead == 0)
		throw Fall::Exception("Unable to read data because the connection has been closed");
	return bytesRead;
}

void SslSocket::read(ByteBuffer & buffer)
{
	buffer.resize(defaultBufferSize);
	std::size_t bytesRead = read(buffer.data(), buffer.size());
	buffer.resize(bytesRead);
}

void SslSocket::write(const void * buffer, std::size_t size)
{
	throw Fall::Exception("Not implemented");
}

void SslSocket::write(const ByteBuffer & buffer)
{
	throw Fall::Exception("Not implemented");
}

void SslSocket::initializeSsl()
{
	std::lock_guard<std::mutex> lock(sslInitializationMutex);
	if(!sslHasBeenInitialized)
	{
		::SSL_library_init();
		::SSL_load_error_strings();
		sslHasBeenInitialized = true;
	}
}

void SslSocket::throwErrnoException(const std::string & message)
{
	std::string exceptionMessage = (message + ": ") + ::strerror(errno);
	throw Fall::Exception(exceptionMessage);
}