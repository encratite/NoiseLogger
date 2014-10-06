#pragma once

#include <Fall/Exception.hpp>

class AlsaException: Fall::Exception
{
public:
	AlsaException(std::string const & functionName, int errorCode);
};