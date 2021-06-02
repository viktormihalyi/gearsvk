#include "PythonDict.h"
#include "stdafx.h"

#include "core/Response.h"
#include "core/Sequence.h"
#include "core/filter/SpatialFilter.h"
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


void PyResponse::registerCallback (uint msg, pybind11::object callback)
{
    for (auto& o : callbacks[msg])
        if (o == callback)
            return;
    callbacks[msg].push_back (callback);
}
