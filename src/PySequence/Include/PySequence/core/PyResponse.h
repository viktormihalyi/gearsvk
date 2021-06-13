#ifndef PYRESPONSE_HPP
#define PYRESPONSE_HPP

#include "PySequence/PySequenceAPI.hpp"

#include "Sequence/Response.h"

#include <memory>

#include <algorithm>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "pybind11/pybind11.h"

class Sequence;
class SpatialFilter;

//! A structure that contains all stimulus parameters.
class PYSEQUENCE_API PyResponse : public Response {
public:
    using Response::Response;
    
    pybind11::object pythonObject;
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject () const;

    pybind11::object joiner;
    pybind11::object setJoiner (pybind11::object joiner);


    std::map<uint32_t, std::vector<pybind11::object>> callbacks;
    void                                          registerCallback (uint32_t msg, pybind11::object callback);

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

#endif
