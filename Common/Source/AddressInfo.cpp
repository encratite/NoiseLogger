#include <Common/AddressInfo.hpp>

#include <cstring>

AddressInfo::AddressInfo():
	_addressInfo(nullptr)
{
}

AddressInfo::AddressInfo(const char * host, uint16_t port):
	_addressInfo(nullptr)
{
	resolve(host, port);
}

AddressInfo::AddressInfo(const std::string & host, uint16_t port):
	_addressInfo(nullptr)
{
	resolve(host, port);
}

AddressInfo::~AddressInfo()
{
	if(_addressInfo != nullptr)
	{
		freeaddrinfo(_addressInfo);
	}
}

void AddressInfo::resolve(const char * host, uint16_t port)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(host == nullptr)
		hints.ai_flags = AI_PASSIVE;
	std::string portString = std::to_string(port);
	int result = getaddrinfo(host, portString.c_str(), &hints, &_addressInfo);
	if(result != 0)
	{
		std::string message = "Failed to resolve host: ";
		message += gai_strerror(result);
		throw Fall::Exception(message);
	}
}

void AddressInfo::resolve(const std::string & host, uint16_t port)
{
	resolve(host.c_str(), port);
}

const addrinfo & AddressInfo::getAddress() const
{
	return *_addressInfo;
}