#pragma once

#include "Sequence/Response.h"

#include "stdafx.h"
#include <memory>

#include <algorithm>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class Sequence;
class SpatialFilter;

//! A structure that contains all stimulus parameters.
class PyResponse : public Response {
public:
    using Response::Response;
    
    GEARS_SHARED_CREATE (PyResponse);

    pybind11::object pythonObject;
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject () const;

    pybind11::object joiner;
    pybind11::object setJoiner (pybind11::object joiner);


    std::map<uint, std::vector<pybind11::object>> callbacks;
    void                                          registerCallback (uint msg, pybind11::object callback);

#if 0
    template<typename T>
    bool executeCallbacks (typename T::P wevent) const
    {
        bool handled = false;
        auto ic      = callbacks.find (T::typeId);
        if (ic != callbacks.end ())
            for (auto& cb : ic->second) {
                pybind11::object rhandled = cb (wevent);
                bool             exb      = rhandled.cast<bool> ();
                if (exb.check ())
                    handled = handled || exb;
            }
        return handled;
    }
#endif
};
