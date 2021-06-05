#include "PythonDict.h"

#include "core/PyResponse.h"
#include "core/PySequence.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>



pybind11::object PyResponse::setPythonObject (pybind11::object o)
{
    pythonObject = o;
    return pythonObject;
}


pybind11::object PyResponse::getPythonObject () const
{
    return pythonObject;
}


pybind11::object PyResponse::setJoiner (pybind11::object joiner)
{
    this->joiner = joiner;
    return this->joiner;
}


void PyResponse::registerCallback (uint32_t msg, pybind11::object callback)
{
    for (auto& o : callbacks[msg])
        if (o.is (callback))
            return;
    callbacks[msg].push_back (callback);
}
