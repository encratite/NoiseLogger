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
	close();
}
	
void SslSocket::connect(const std::string & host, uint16_t port, const std::string & certificatePath)
{
	if(_socket != invalidSocket)
		throw Fall::Exception("Unable to connect because the socket is already in use");
	close();
	hostent * hostEntity = gethostbyname(host.c_str());
	if(hostEntity == nullptr)
		closeAndThrow("Failed to resolve host name");
	createSocket();
	sockaddr_in address = getAddress(*reinterpret_cast<unsigned long *>(hostEntity->h_addr_list[0]), port);
	int result = ::connect(_socket, reinterpret_cast<sockaddr *>(&address), sizeof(sockaddr));
	if(result == -1)
		closeAndThrowErrno("Failed to connect to server");
	createSslContext(certificatePath);
	SSL_set_connect_state(_ssl);
	result = SSL_connect(_ssl);
	if(result != 1)
		closeAndThrowSsl("Failed to perform SSL connection initialization");
}

void SslSocket::bindAndListen(uint16_t port, const std::string & certificatePath)
{
	if(_socket != invalidSocket)
		throw Fall::Exception("Unable to bind socket because it is already in use");
	close();
	throw Fall::Exception("Not implemented");
}

void SslSocket::close()
{
	if(_ssl != nullptr)
	{
		SSL_free(_ssl);
		_ssl = nullptr;
	}
	if(_sslContext != nullptr)
	{
		SSL_CTX_free(_sslContext);
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
	checkSocket();
	ssize_t bytesRead = recv(_socket, buffer, bufferSize, 0);
	if(bytesRead == -1)
		closeAndThrowErrno("Failed to read from socket");
	else if(bytesRead == 0)
		closeAndThrow("Failed to read from socket because the connection has been closed");
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
	checkSocket();
	ssize_t bytesSent = send(_socket, buffer, size, 0);
	if(bytesSent == -1)
		closeAndThrowErrno("Failed to write to socket");
	else if(bytesSent < static_cast<ssize_t>(size))
		closeAndThrow("Failed to write to socket because the connection has been closed");
}

void SslSocket::write(const ByteBuffer & buffer)
{
	write(buffer.data(), buffer.size());
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

void SslSocket::checkSocket()
{
	if(_socket == invalidSocket)
		throw Fall::Exception("Socket has not been initialized");
}

void SslSocket::createSocket()
{
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(_socket == invalidSocket)
		closeAndThrowErrno("Failed to create socket");
}

sockaddr_in SslSocket::getAddress(unsigned long address, uint16_t port)
{
	sockaddr_in output;
	output.sin_family = AF_INET;
	output.sin_addr.s_addr = address;
	output.sin_port = htons(port);
	memset(&(output.sin_zero), 0, sizeof(output.sin_zero));
	return output;
}

void SslSocket::createSslContext(const std::string & certificatePath)
{
	auto method = TLSv1_1_client_method();
	_sslContext = SSL_CTX_new(method);
	if(_sslContext == nullptr)
		closeAndThrow("Failed to create SSL context");
	SSL_CTX_set_verify(_sslContext, SSL_VERIFY_PEER, nullptr);
	int result = SSL_CTX_use_certificate_file(_sslContext, certificatePath.c_str(), SSL_FILETYPE_PEM);
	if(result != 1)
		closeAndThrowSsl("Unable to load certificate file");
	result = SSL_CTX_use_PrivateKey_file(_sslContext, certificatePath.c_str(), SSL_FILETYPE_PEM);
	if(result != 1)
		closeAndThrowSsl("Unable to load private key file");
	_ssl = SSL_new(_sslContext);
	if(_ssl == nullptr)
		closeAndThrow("Failed to create SSL structure");
	result = SSL_set_fd(_ssl, _socket);
	if(result == 0)
		closeAndThrow("Failed to set SSL file descriptor");
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