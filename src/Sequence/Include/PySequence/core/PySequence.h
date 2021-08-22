#ifndef PYSEQUENCE_HPP
#define PYSEQUENCE_HPP

#include "Sequence/SequenceAPI.hpp"

#include "Sequence/Sequence.h"

#include <memory>
#include <string>

#include "pybind11/pybind11.h"

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>

class Stimulus;
class Response;

//! A structure that contains all sequence parameters.
class SEQUENCE_API PySequence : public Sequence {
private:

    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    PySequence ();
    PySequence (std::string name);
    virtual ~PySequence ();

    pybind11::object set (pybind11::object settings);

    pybind11::object          onReset (pybind11::object cb);
    std::shared_ptr<PySequence> setAgenda (pybind11::object agenda);

    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject ();

    virtual void OnStimulusAdded (std::shared_ptr<Stimulus> stimulus) override;

    template<typename Archive>
    void serialize (Archive& ar)
    {
        ar (cereal::base_class<Sequence> (this));
    }
};

CEREAL_REGISTER_TYPE (PySequence)
CEREAL_REGISTER_POLYMORPHIC_RELATION (Sequence, PySequence)

#endif