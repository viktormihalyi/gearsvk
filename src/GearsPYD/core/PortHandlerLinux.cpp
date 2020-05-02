
#ifdef __linux__

#include <fcntl.h>
#include <unistd.h>
#include "PortHandler.h"

PortHandler::PortHandler()
{
	port = -1;
}
PortHandler::PortHandler(const char* name)
{
	port = open(name, O_RDWR | O_NOCTTY | O_SYNC);
}

PortHandler::PortHandler(PortHandler&& o)
{
	port = o.port;
	o.port = -1;
}

void PortHandler::setCommand(int cmd)
{
    // TODO: send signal in linux
}

bool PortHandler::isInvalid()
{
	return port < 0;
}

void PortHandler::close()
{
	if (port >= 0)
	{
		::close(port);
		port = -1;
	}
}

#endif