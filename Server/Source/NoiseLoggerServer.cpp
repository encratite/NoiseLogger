#include <iostream>

#include <Server/NoiseLoggerServer.hpp>

NoiseLoggerServer::NoiseLoggerServer(const ServerConfiguration & configuration):
	_configuration(configuration)
{
	_sslServer.onNewClient = std::bind(&NoiseLoggerServer::onNewClient, this, std::placeholders::_1);
}
	
void NoiseLoggerServer::run()
{
	_sslServer.run(_configuration.serverPort, _configuration.certificatePath);
}

void NoiseLoggerServer::onNewClient(SslClientPointer client)
{
	try
	{
		std::cout << "Client connected: " << client->getAddress() << std::endl;
	}
	catch(const std::exception & exception)
	{
		std::cout << "Client error: " << exception.what() << std::endl;
	}
}