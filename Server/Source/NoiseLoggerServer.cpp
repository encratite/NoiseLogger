#include <iostream>
#include <cstring>

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
	const std::size_t lengthPrefixSize = sizeof(uint16_t);
	try
	{
		std::string address = client->getAddress();
		auto writeLine = [&](const std::string & message)
		{
			Fall::log("[" + address + "] " + message);
		};
		writeLine("Client connected");
		ByteBuffer buffer;
		while(true)
		{
			client->read(buffer);
			if(buffer.size() < lengthPrefixSize)
				continue;
			std::size_t offset = 0;
			uint16_t lengthPrefix = deserializeUInt16(buffer, offset);
			std::size_t packetSize = lengthPrefixSize + lengthPrefix;
			if(buffer.size() < packetSize)
				continue;
			ByteBuffer logPacketBuffer(packetSize - lengthPrefixSize);
			std::memcpy(logPacketBuffer.data(), buffer.data() + lengthPrefixSize, logPacketBuffer.size());
			LogPacket logPacket(logPacketBuffer);
			ByteBuffer newBuffer(buffer.size() - packetSize);
			std::memcpy(newBuffer.data(), buffer.data() + packetSize, newBuffer.size());
			buffer = newBuffer;
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