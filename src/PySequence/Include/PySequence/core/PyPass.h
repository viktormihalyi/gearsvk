#ifndef PYPASS_HPP
#define PYPASS_HPP

#include "PySequence/PySequenceAPI.hpp"

#include "Sequence/Pass.h"

#include <algorithm>
#include <glm/glm.hpp>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "pybind11/pybind11.h"

class Sequence;
class Stimulus;

//! A structure that specifies a shape in a stimulus.
class PYSEQUENCE_API PyPass : public Pass {
public:

    using Pass::Pass;

    pybind11::object joiner;
    pybind11::object setJoiner (pybind11::object joiner);

    pybind11::object pythonObject;
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject ();

    void setPolygonMask (std::string mode, pybind11::object o);
};

#endif