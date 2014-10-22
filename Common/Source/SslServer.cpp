#include <Common/SslServer.hpp>

void SslServer::bindAndListen(uint16_t port, const std::string & certificatePath)
{
	if(!isInvalidSocket())
		throw Fall::Exception("Unable to bind socket because it is already in use");
	close();
	AddressInfo addressInfo(nullptr, port);
	const addrinfo & address = addressInfo.getAddress();
	createSocket(address);
	int error = bind(_socket, address.ai_addr, address.ai_addrlen);
	if(error)
		closeAndThrowErrno("Failed to bind socket");
	error = listen(_socket, SOMAXCONN);
	if(error)
		closeAndThrowErrno("Failed to listen for connections");
	createSslContext(false, certificatePath);
	SSL_set_accept_state(_ssl);
}