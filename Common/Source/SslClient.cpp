#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <Common/SslClient.hpp>

namespace
{
	const std::size_t defaultBufferSize = 4 * 1024;
}

void SslClient::connect(const std::string & host, uint16_t port, const std::string & certificatePath, const std::string & certificateAuthorityPath)
{
	if(!isInvalidSocket())
		throw Fall::Exception("Unable to connect because the socket is already in use");
	close();
	AddressInfo addressInfo(host, port);
	const addrinfo & address = addressInfo.getAddress();
	createSocket(address);
	int result = ::connect(_socket, address.ai_addr, address.ai_addrlen);
	if(result != 0)
		closeAndThrowErrno("Failed to connect to server");
	createSslContext(true, certificatePath, certificateAuthorityPath);
	SSL_set_connect_state(_ssl);
	result = SSL_connect(_ssl);
	if(result != 1)
		closeAndThrowSsl("Failed to perform SSL connection initialization");
}

std::size_t SslClient::read(void * buffer, std::size_t bufferSize)
{
	checkSocket();
	ssize_t bytesRead = recv(_socket, buffer, bufferSize, 0);
	if(bytesRead == -1)
		closeAndThrowErrno("Failed to read from socket");
	else if(bytesRead == 0)
		closeAndThrow("Failed to read from socket because the connection has been closed");
	return bytesRead;
}

void SslClient::read(ByteBuffer & buffer)
{
	uint8_t readBuffer[defaultBufferSize];
	std::size_t bytesRead = read(&readBuffer, sizeof(readBuffer));
	std::size_t bufferSize = buffer.size();
	buffer.resize(bufferSize + bytesRead);
	std::memcpy(buffer.data() + bufferSize, &readBuffer, bytesRead);
}

void SslClient::write(const void * buffer, std::size_t size)
{
	checkSocket();
	ssize_t bytesSent = send(_socket, buffer, size, 0);
	if(bytesSent == -1)
		closeAndThrowErrno("Failed to write to socket");
	else if(bytesSent < static_cast<ssize_t>(size))
		closeAndThrow("Failed to write to socket because the connection has been closed");
}

void SslClient::write(const ByteBuffer & buffer)
{
	write(buffer.data(), buffer.size());
}

bool SslClient::isConnected()
{
	return !isInvalidSocket();
}

std::string SslClient::getAddress()
{
	sockaddr_storage addressStorage;
	socklen_t length = sizeof(addressStorage);
	int result = getpeername(_socket, reinterpret_cast<sockaddr *>(&addressStorage), &length);
	if(result != 0)
		closeAndThrowErrno("Failed to retrieve peer name");
	char buffer[INET6_ADDRSTRLEN];
	void * source;
	if(addressStorage.ss_family == AF_INET)
	{
		sockaddr_in * address = reinterpret_cast<sockaddr_in *>(&addressStorage);
		source = &address->sin_addr;
	}
	else
	{
		sockaddr_in6 * address = reinterpret_cast<sockaddr_in6 *>(&addressStorage);
		source = &address->sin6_addr;
	}
	inet_ntop(addressStorage.ss_family, source, buffer, sizeof(buffer));
	std::string output(buffer);
	return buffer;
}

void SslClient::setClientData(int socket, SSL_CTX * sslContext)
{
	close();
	_socket = socket;
	_sslContext = sslContext;
	createSslStructure();
}

SSL * SslClient::getSslStructure()
{
	return _ssl;
}