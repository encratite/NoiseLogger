#pragma once

#include <Fall/Exception.hpp>

class AlsaException: Fall::Exception
{
public:
	AlsaException(const std::string & functionName, int errorCode);
};