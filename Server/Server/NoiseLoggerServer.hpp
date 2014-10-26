#pragma once

#include <libpq-fe.h>

#include <Common/SslServer.hpp>
#include <Server/ServerConfiguration.hpp>

class NoiseLoggerServer
{
public:
	NoiseLoggerServer(const ServerConfiguration & configuration);
	~NoiseLoggerServer();
	
	void run();
	
private:
	ServerConfiguration _configuration;
	SslServer _sslServer;
	PGconn * _databaseConnection;
	
	void onNewClient(SslClientPointer client);
	void checkDatabaseStatus();
	void checkDatabaseResultStatus(PGresult * result);
};