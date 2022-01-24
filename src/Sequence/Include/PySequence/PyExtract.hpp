#ifndef PYEXTRACT_HPP
#define PYEXTRACT_HPP

#include <pybind11/pybind11.h>

#include "RenderGraph/Utils/Assert.hpp"


template<typename T>
class PyExtract {
private:
    T    casted;
    bool ok;

public:
    PyExtract (pybind11::object obj)
    {
        try {
            casted = obj.cast<T> ();
            ok     = true;
        } catch (pybind11::cast_error&) {
            ok = false;
        }
    }

    ~PyExtract () = default;

    bool check () const { return ok; }

    T operator() ()
    {
        GVK_ASSERT (ok);
        return casted;
    }
};

#endif