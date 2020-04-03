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

Response::Response ()
    : question ("Pizza?"), loop (false), duration (1), startingFrame (0)
{
}


pybind11::object Response::setPythonObject (pybind11::object o)
{
    pythonObject = o;
    return pythonObject;
}

pybind11::object Response::getPythonObject () const
{
    return pythonObject;
}


pybind11::object Response::setJoiner (pybind11::object joiner)
{
    this->joiner = joiner;
    return this->joiner;
}

void Response::registerCallback (uint msg, pybind11::object callback)
{
    for (auto& o : callbacks[msg])
        if (o == callback)
            return;
    callbacks[msg].push_back (callback);
}


void Response::addButton (std::string label, float x, float y, float w, float h, uint key, bool visible)
{
    Button b = {label, x, y, w, h, key, visible};
    buttons.push_back (b);
}