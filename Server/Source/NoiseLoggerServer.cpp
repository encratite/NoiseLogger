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
	DEBUG_MARK
	try
	{
		DEBUG_MARK
		NoiseLoggerServerClient serverClient(client);
		DEBUG_MARK
		serverClient.log("Client connected");
		DEBUG_MARK
		while(true)
		{
			LogPacket logPacket;
			DEBUG_MARK
			serverClient.readPacket(logPacket);
			serverClient.log("Received packet from client");
			storePacketInDatabase(logPacket, serverClient);
			DEBUG_MARK
		}
	}
	catch(const std::exception & exception)
	{
		Fall::log(std::string("Client error: ") + exception.what());
	}
	DEBUG_MARK
}

void NoiseLoggerServer::storePacketInDatabase(const LogPacket & logPacket, const NoiseLoggerServerClient & client)
{
	const char * query = "select insert_volume($1::timestamp, $2::int4, $3::text)";
	const int parameterCount = 3;
	const Oid * parameterTypes = nullptr;
	int textFormat = 0;
	int binaryFormat = 1;
	uint64_t timestamp = logPacket.initialTimestamp;
	const std::string & address = client.getAddress();
	int parameterLengths[parameterCount] =
	{
		sizeof(uint64_t),
		sizeof(uint32_t),
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
		uint32_t parameterSample = static_cast<uint32_t>(htobe16(sample));
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
		message = Fall::trim(message);
		throw Fall::Exception(message);
	}
}

void NoiseLoggerServer::checkDatabaseResultStatus(PGresult * result)
{
	if(PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		std::string message = "Database query error: ";
		message += PQerrorMessage(_databaseConnection);
		message = Fall::trim(message);
		PQclear(result);
		throw Fall::Exception(message);
	}
}