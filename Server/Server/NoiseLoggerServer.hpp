#pragma once

#include <Common/SslServer.hpp>
#include <Server/ServerConfiguration.hpp>

class NoiseLoggerServer
{
public:
	NoiseLoggerServer(const ServerConfiguration & configuration);
	
	void run();
	
private:
	ServerConfiguration _configuration;
	SslServer _sslServer;
	
	void onNewClient(SslClientPointer client);
};