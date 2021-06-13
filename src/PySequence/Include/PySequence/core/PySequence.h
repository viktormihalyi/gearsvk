#ifndef PYSEQUENCE_HPP
#define PYSEQUENCE_HPP

#include "PySequence/PySequenceAPI.hpp"

#include "Sequence/Sequence.h"

#include <map>
#include <string>

#include <memory>
#include <string>

#include "pybind11/pybind11.h"

class Stimulus;
class Response;

//! A structure that contains all sequence parameters.
class PYSEQUENCE_API PySequence : public Sequence {
private:
    using Sequence::Sequence;

public:
    pybind11::object set (pybind11::object settings);

    pybind11::object          resetCallback;
    pybind11::object          onReset (pybind11::object cb);
    std::shared_ptr<PySequence> setAgenda (pybind11::object agenda);

    pybind11::object pythonObject;
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject ();

    virtual void OnStimulusAdded (std::shared_ptr<Stimulus> stimulus) override;
};

#endif