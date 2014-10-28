#include <iostream>

#include <endian.h>

#include <Fall/Console.hpp>
#include <Fall/Time.hpp>

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
	const char * query = "select insert_volume($1::timestamp, $2::integer, $3::text)";
	const int parameterCount = 3;
	const Oid * parameterTypes = nullptr;
	int textFormat = 0;
	int binaryFormat = 1;
	uint64_t timestamp = logPacket.initialTimestamp;
	const std::string & address = client.getAddress();
	int parameterLengths[parameterCount] =
	{
		sizeof(uint64_t),
		sizeof(uint16_t),
		static_cast<int>(address.length())
	};
	int parameterFormats[parameterCount] =
	{
		binaryFormat,
		binaryFormat,
		textFormat
	};
	for(uint16_t sample : logPacket.samples)
	{
		uint64_t parameterTimestamp = htobe64(timestamp);
		uint16_t parameterSample = htobe16(sample);
		const char * parameterValues[parameterCount] =
		{
			reinterpret_cast<char *>(&parameterTimestamp),
			reinterpret_cast<char *>(&parameterSample),
			address.c_str()
		};
		PGresult * result = PQexecParams(_databaseConnection, query, parameterCount, parameterTypes, parameterValues, parameterLengths, parameterFormats, binaryFormat);
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