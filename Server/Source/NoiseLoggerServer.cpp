#include <iostream>

#include <Fall/Console.hpp>
#include <Fall/Time.hpp>

#include <Common/LogPacket.hpp>
#include <Common/Serialization.hpp>
#include <Server/NoiseLoggerServer.hpp>
#include <Server/NoiseLoggerServerClient.hpp>

NoiseLoggerServer::NoiseLoggerServer(const ServerConfiguration & configuration):
	_configuration(configuration),
	_databaseConnection(nullptr)
{
	_sslServer.onNewClient = [&](SslClientPointer client) { onNewClient(client); };
}

NoiseLoggerServer::~NoiseLoggerServer()
{
	if(_databaseConnection != nullptr)
	{
		PQfinish(_databaseConnection);
		_databaseConnection = nullptr;
	}
}
	
void NoiseLoggerServer::run()
{
	_databaseConnection = PQconnectdb(_configuration.databaseConnectionString.c_str());
	checkDatabaseStatus();
	_sslServer.run(_configuration.serverPort, _configuration.certificatePath);
}

void NoiseLoggerServer::onNewClient(SslClientPointer client)
{
	try
	{
		NoiseLoggerServerClient serverClient(client);
		serverClient.log("Client connected");
		while(true)
		{
			LogPacket logPacket;
			serverClient.readPacket(logPacket);
			throw Fall::Exception("Not implemented");
		}
	}
	catch(const std::exception & exception)
	{
		Fall::log(std::string("Client error: ") + exception.what());
	}
}

void NoiseLoggerServer::checkDatabaseStatus()
{
	if(PQstatus(_databaseConnection) != CONNECTION_OK)
	{
		std::string message = "Database error: ";
		message += PQerrorMessage(_databaseConnection);
		throw Fall::Exception(message);
	}
}

void NoiseLoggerServer::checkDatabaseResultStatus(PGresult * result)
{
	if(PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		std::string message = "Database query error: ";
		message += PQerrorMessage(_databaseConnection);
		PQclear(result);
		throw Fall::Exception(message);
	}
}