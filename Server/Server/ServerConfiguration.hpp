#pragma once

#include <cstdint>
#include <string>

struct ServerConfiguration
{
	// The TCP port the server runs on
	uint16_t serverPort;
	// Path to the server certificate/key
	std::string certificatePath;
	// Path to the CA certificate
	std::string certificateAuthorityPath;
	// PostgreSQL connection string
	std::string databaseConnectionString;
};