#include <Common/SslServer.hpp>

SslServer::SslServer():
	_running(false)
{
}

SslServer::~SslServer()
{
	_running = false;
}

void SslServer::run(uint16_t port, const std::string & certificatePath)
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
	_running = true;
	while(_running)
	{
		sockaddr_storage clientAddress;
		socklen_t clientAddressSize = sizeof(clientAddress);
		int clientSocket = accept(_socket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressSize);
		if(clientSocket < 0)
		{
			if(!_running)
				break;
			closeAndThrowErrno("Failed to accept connection");
		}
		SslClientPointer client(new SslClient);
		client->setClientData(clientSocket, _sslContext);
		int result = SSL_accept(client->getSslStructure());
		if(result != 1)
		{
			continue;
		}
		if(onNewClient != nullptr)
			onNewClient(client);
	}
}

void SslServer::stop()
{
	_running = false;
	close();
}