#ifndef PYPASS_HPP
#define PYPASS_HPP

#include "Sequence/SequenceAPI.hpp"

#include "Sequence/Pass.h"

#include <memory>

#include "pybind11/pybind11.h"

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>

class Sequence;
class Stimulus;

//! A structure that specifies a shape in a stimulus.
class SEQUENCE_API PyPass : public Pass {
private:

    struct Impl;
    std::unique_ptr<Impl> impl;

public:

    PyPass ();
    virtual ~PyPass ();

    pybind11::object setJoiner (pybind11::object joiner);
    pybind11::object getJoiner ();

    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject ();

    void setPolygonMask (std::string mode, pybind11::object o);

    template <typename Archive>
    void serialize (Archive& ar)
    {
        ar (cereal::base_class<Pass> (this));
    }
};

CEREAL_REGISTER_TYPE (PyPass)
CEREAL_REGISTER_POLYMORPHIC_RELATION (Pass, PyPass)


#endif