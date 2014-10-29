#include <iostream>

#include <endian.h>

#include <Fall/Console.hpp>
#include <Fall/String.hpp>
#include <Fall/Time.hpp>

#include <Common/Debug.hpp>
#include <Common/LogPacket.hpp>
#include <Common/Serialization.hpp>
#include <Server/NoiseLoggerServer.hpp>

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
	Fall::log("Running server on port " + std::to_string(_configuration.serverPort));
	_sslServer.run(_configuration.serverPort, _configuration.certificatePath, _configuration.certificateAuthorityPath);
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
			serverClient.log("Received packet from client");
			storePacketInDatabase(logPacket, serverClient);
		}
	}
	catch(const std::exception & exception)
	{
		Fall::log(std::string("Client error: ") + exception.what());
	}
}

void NoiseLoggerServer::storePacketInDatabase(const LogPacket & logPacket, const NoiseLoggerServerClient & client)
{
	const char * query = "select insert_volume(timestamp 'epoch' + ($1::bigint * interval '1 millisecond'), $2::integer, $3)";
	const int parameterCount = 3;
	int binaryFormat = 1;
	uint64_t timestamp = logPacket.initialTimestamp;
	const std::string & address = client.getAddress();
	for(uint16_t sample : logPacket.samples)
	{
		std::string timestampString = std::to_string(timestamp);
		std::string sampleString = std::to_string(sample);
		const char * parameterValues[parameterCount] =
		{
			timestampString.c_str(),
			sampleString.c_str(),
			address.c_str()
		};
		PGresult * result = PQexecParams(_databaseConnection, query, parameterCount, nullptr, parameterValues, nullptr, nullptr, binaryFormat);
		checkDatabaseResultStatus(result);
		timestamp += logPacket.interval;
	}
}

void NoiseLoggerServer::checkDatabaseStatus()
{
	if(PQstatus(_databaseConnection) != CONNECTION_OK)
	{
		std::string message = "Database error: ";
		message += PQerrorMessage(_databaseConnection);
		message = Fall::trim(message);
		throw Fall::Exception(message);
	}
}

void NoiseLoggerServer::checkDatabaseResultStatus(PGresult * result)
{
	if(PQresultStatus(result) != PGRES_TUPLES_OK)
	{
		std::string message = "Database query error: ";
		message += PQerrorMessage(_databaseConnection);
		message = Fall::trim(message);
		PQclear(result);
		throw Fall::Exception(message);
	}
}