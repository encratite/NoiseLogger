#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <Fall/Exception.hpp>

class AddressInfo
{
public:
	AddressInfo();
	AddressInfo(const char * host, uint16_t port);
	AddressInfo(const std::string & host, uint16_t port);
	~AddressInfo();
	
	void resolve(const char * host, uint16_t port);
	void resolve(const std::string & host, uint16_t port);
	
	const addrinfo & getAddress() const;
	
private:
	addrinfo * _addressInfo;
};