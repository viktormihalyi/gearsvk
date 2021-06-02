#pragma once

#include "Sequence/Sequence.h"

#include <map>
#include <string>

#include "stdafx.h"
#include <memory>
#include <string>

class Stimulus;
class Response;

//! A structure that contains all sequence parameters.
class PySequence : public Sequence {
private:
    using Sequence::Sequence;

public:
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (PySequence);

    pybind11::object set (pybind11::object settings);

    pybind11::object          resetCallback;
    pybind11::object          onReset (pybind11::object cb);
    std::shared_ptr<PySequence> setAgenda (pybind11::object agenda);

    pybind11::object pythonObject;
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject ();
};
