#pragma once

#include <Common/SslSocket.hpp>
#include <Common/SslClient.hpp>

class SslServer: public SslSocket
{
public:
	SslServer();
	~SslServer();

	void run(uint16_t port, const std::string & certificatePath);
	void stop();

	virtual void onNewClient(const SslClientPointer & client) = 0;

private:
	bool _running;
};