#pragma once

#include "Sequence/Pass.h"

#include "stdafx.h"

#include <algorithm>
#include <glm/glm.hpp>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>

class Sequence;
class Stimulus;

//! A structure that specifies a shape in a stimulus.
class PyPass : public Pass {
public:

    using Pass::Pass;

    pybind11::object joiner;
    pybind11::object setJoiner (pybind11::object joiner);


public:
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (PyPass);

    pybind11::object pythonObject;
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject ();

    void setPolygonMask (std::string mode, pybind11::object o);
};
