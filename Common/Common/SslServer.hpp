#pragma once

#include <Common/SslSocket.hpp>

class SslServer: public SslSocket
{
public:
	void bindAndListen(uint16_t port, const std::string & certificatePath);
};