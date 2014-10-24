#pragma once

#include <functional>

#include <Common/SslSocket.hpp>
#include <Common/SslClient.hpp>

typedef std::function<void (const SslClientPointer & client)> OnNewClientEvent;

class SslServer: public SslSocket
{
public:
	OnNewClientEvent onNewClient;
	
	SslServer();
	~SslServer();

	void run(uint16_t port, const std::string & certificatePath);
	void stop();

private:
	bool _running;
};