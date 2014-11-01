#include <chrono>

#include <Common/Debug.hpp>
#include <Common/SslServer.hpp>

SslServer::SslServer():
	_running(false)
{
}

SslServer::~SslServer()
{
	stop();
	for(ClientFuture & future : _clientThreads)
		future.wait();
}

void SslServer::run(uint16_t port, const std::string & certificatePath, const std::string & certificateAuthorityPath)
{
	if(!isInvalidSocket())
		throw Fall::Exception("Unable to bind socket because it is already in use");
	close();
	AddressInfo addressInfo(nullptr, port);
	const addrinfo & address = addressInfo.getAddress();
	createSocket(address);
	int option = 1;
	int result = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if(result != 0)
		closeAndThrowErrno("Failed to set socket option");
	result = bind(_socket, address.ai_addr, address.ai_addrlen);
	if(result != 0)
		closeAndThrowErrno("Failed to bind socket");
	result = listen(_socket, SOMAXCONN);
	if(result != 0)
		closeAndThrowErrno("Failed to listen for connections");
	createSslContext(false, certificatePath, certificateAuthorityPath);
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
		SslClient * client = new SslClient;
		client->setClientData(clientSocket, _sslContext);
		int result = SSL_accept(client->getSslStructure());
		if(result != 1)
		{
			delete client;
			continue;
		}
		if(onNewClient != nullptr)
		{
			ClientFuture future = std::async(std::launch::async, [&]()
				{
					SslClientPointer clientPointer(client);
					onNewClient(clientPointer);
				}
			);
			_clientThreads.push_back(std::move(future));
		}
		cleanUpThreads();
	}
}

void SslServer::stop()
{
	_running = false;
	close();
}

void SslServer::cleanUpThreads()
{
	std::chrono::seconds duration(0);
	for(auto i = _clientThreads.begin(); i != _clientThreads.end();)
	{
		std::future_status status = i->wait_for(duration);
		if(status == std::future_status::ready)
			i = _clientThreads.erase(i);
		else
			i++;
	}
}