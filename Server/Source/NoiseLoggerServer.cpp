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

void NoiseLoggerServer::onNewClient(const SslClientPointer & client)
{
}