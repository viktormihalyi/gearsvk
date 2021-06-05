#include "PythonDict.h"

#include "core/PyResponse.h"
#include "core/PySequence.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

struct PyResponse::Impl {
    pybind11::object pythonObject;
    pybind11::object joiner;
    std::map<uint32_t, std::vector<pybind11::object>> callbacks;
};


PyResponse::PyResponse ()
    : impl { std::make_unique<Impl> () }
{
}


pybind11::object PyResponse::setPythonObject (pybind11::object o)
{
    impl->pythonObject = o;
    return impl->pythonObject;
}


pybind11::object PyResponse::getPythonObject () const
{
    return impl->pythonObject;
}


pybind11::object PyResponse::setJoiner (pybind11::object joiner)
{
    impl->joiner = joiner;
    return impl->joiner;
}


void PyResponse::registerCallback (uint32_t msg, pybind11::object callback)
{
    for (auto& o : impl->callbacks[msg])
        if (o.is (callback))
            return;
    impl->callbacks[msg].push_back (callback);
}
