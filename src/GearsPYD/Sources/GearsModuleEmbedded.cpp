#include "GearsModuleCommon.hpp"

// we are supposed to be able to import embedded modules from python, but it doesnt work
// https://pybind11.readthedocs.io/en/stable/advanced/embedding.html#adding-embedded-modules
//
// tests use this embedded module

PYBIND11_EMBEDDED_MODULE (GearsModule, m)
{
    FillModule (m);
}
