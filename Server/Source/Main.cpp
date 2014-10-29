#include <iostream>

#include <Fall/Configuration.hpp>
#include <Fall/Exception.hpp>

#include <Common/Debug.hpp>
#include <Server/NoiseLoggerServer.hpp>
#include <Server/ServerConfiguration.hpp>

namespace
{
	const std::string configurationPath("NoiseLoggerServer.conf");
}

void runServer()
{
	Fall::Configuration configuration(configurationPath);
	ServerConfiguration serverConfiguration;
	serverConfiguration.serverPort = configuration.getNumber<uint16_t>("serverPort");
	serverConfiguration.certificatePath = configuration.getString("certificatePath");
	serverConfiguration.certificateAuthorityPath = configuration.getString("certificateAuthorityPath");
	serverConfiguration.databaseConnectionString = configuration.getString("databaseConnectionString");
	NoiseLoggerServer server(serverConfiguration);
	server.run();
}

int main(int argc, char ** argv)
{
	try
	{
		runServer();
	}
	catch(const Fall::Exception & exception)
	{
		std::cerr << "Unhandled application exception: " << exception.getMessage() << std::endl;
		return 1;
	}
	catch(const std::exception & exception)
	{
		std::cerr << "Unhandled standard exception: " << exception.what() << std::endl;
		return 2;
	}
	catch(...)
	{
		std::cerr << "Unhandled exception of unknown type" << std::endl;
		return 3;
	}
	return 0;
}