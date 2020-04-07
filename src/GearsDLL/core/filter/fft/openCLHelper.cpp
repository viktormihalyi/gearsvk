#include "stdafx.h"
#include "openCLCore.h"

namespace OpenCLHelper
{
	void clPrintError( cl_int& errorCode )
	{
#ifdef _DEBUG
		if ( !errorCode )
			return;

		std::cout << std::endl << "*********************************************************************************************" << std::endl << "** Error: ";
		switch ( errorCode )
		{
			case CL_INVALID_CONTEXT:
			std::cout << "Invalid cl context";
			break;
			case CL_INVALID_MEM_OBJECT:
			std::cout << "Invalid cl memory object";
			break;
			case CL_INVALID_WORK_GROUP_SIZE:
			std::cout << "Invalid cl work group size";
			break;
			case CL_INVALID_KERNEL_ARGS:
			std::cout << "Invalid cl kernel args";
			break;
			case CL_INVALID_VALUE:
			std::cout << "Invalid cl value";
			break;
			case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
			std::cout << "Invalid gl sharegroup reference";
			break;
			case CL_INVALID_ARG_INDEX:
			std::cout << "Invaluid argumentum index";
			case -9999:
			std::cout << "Illegal read or write to a buffer";
			break;

		}
		std::cout << " (" << errorCode << ") " << std::endl << "*********************************************************************************************" << std::endl;
		errorCode = 0;
#endif
	}
}