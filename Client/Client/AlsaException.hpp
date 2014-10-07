#pragma once

#include <Fall/Exception.hpp>

class AlsaException: public Fall::Exception
{
public:
	AlsaException(const std::string & functionName, int errorCode);
};