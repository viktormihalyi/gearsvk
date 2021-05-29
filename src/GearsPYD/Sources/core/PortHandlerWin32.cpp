#include "stdafx.h"

#ifdef _WIN32
#include "PortHandler.h"

PortHandler::PortHandler()
{
	port = INVALID_HANDLE_VALUE;
}

PortHandler::PortHandler(const char* name)
{
    port = CreateFile(name,
				GENERIC_READ | GENERIC_WRITE,
				0,
				0,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				0);
}

PortHandler::PortHandler(PortHandler&& o)
{
	port = o.port;
	o.port = INVALID_HANDLE_VALUE;
}

void PortHandler::setCommand(int cmd)
{
    EscapeCommFunction(port, cmd);
}

bool PortHandler::isInvalid()
{
    return port == INVALID_HANDLE_VALUE;
}

void PortHandler::close()
{
	if (port != INVALID_HANDLE_VALUE)
	{
		CloseHandle(port);
		port = INVALID_HANDLE_VALUE;
	}
}

#endif