#include <Common/SslClient.hpp>

namespace
{
	const std::size_t defaultBufferSize = 4 * 1024;
}

void SslClient::connect(const std::string & host, uint16_t port, const std::string & certificatePath)
{
	if(!isInvalidSocket())
		throw Fall::Exception("Unable to connect because the socket is already in use");
	close();
	AddressInfo addressInfo(host, port);
	const addrinfo & address = addressInfo.getAddress();
	createSocket(address);
	int success = ::connect(_socket, address.ai_addr, address.ai_addrlen);
	if(!success)
		closeAndThrowErrno("Failed to connect to server");
	createSslContext(true, certificatePath);
	SSL_set_connect_state(_ssl);
	int result = SSL_connect(_ssl);
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
	buffer.resize(defaultBufferSize);
	std::size_t bytesRead = read(buffer.data(), buffer.size());
	buffer.resize(bytesRead);
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

void SslClient::setClientSocket(int socket, const std::string & certificatePath)
{
	close();
	_socket = socket;
	createSslContext(true, certificatePath);
}