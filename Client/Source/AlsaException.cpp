#include <alsa/asoundlib.h>

#include <Client/AlsaException.hpp>

AlsaException::AlsaException(std::string const & functionName, int errorCode)
{
	std::string alsaMessage(snd_strerror(errorCode));
	_message = functionName + " error: " + alsaMessage;
}