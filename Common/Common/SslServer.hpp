#pragma once

#include <functional>
#include <future>
#include <list>

#include <Common/SslSocket.hpp>
#include <Common/SslClient.hpp>

typedef std::function<void (SslClientPointer client)> OnNewClientEvent;
typedef std::future<void> ClientFuture;

class SslServer: public SslSocket
{
public:
	OnNewClientEvent onNewClient;
	
	SslServer();
	~SslServer();

	void run(uint16_t port, const std::string & certificatePath, const std::string & certificateAuthorityPath);
	void stop();

private:
	bool _running;
	std::list<ClientFuture> _clientThreads;

	void cleanUpThreads();
};