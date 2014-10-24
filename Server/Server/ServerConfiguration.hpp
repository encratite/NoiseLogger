#pragma once

#include <cstdint>
#include <string>

struct ServerConfiguration
{
	// The TCP port the server runs on
	uint16_t serverPort;
	// Path to the server certificate chain/key
	std::string certificatePath;
	// PostgreSQL connection string
	std::string databaseConnectionString;
};