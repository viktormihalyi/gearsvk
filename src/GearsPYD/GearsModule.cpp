#include "GearsModuleCommon.hpp"

#ifndef NDEBUG 
PYBIND11_MODULE (GearsModule, m)
#else
PYBIND11_MODULE (GearsModule_d, m)
#endif
{
    FillModule (m);
}
