#ifndef PYPASS_HPP
#define PYPASS_HPP

#include "Sequence/SequenceAPI.hpp"

#include "Sequence/Pass.h"

#include <memory>

#include "pybind11/pybind11.h"

#ifdef GEARSVK_CEREAL
#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#endif

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

#ifdef GEARSVK_CEREAL
    template<typename Archive>
    void serialize (Archive& ar)
    {
        ar (cereal::base_class<Pass> (this));
    }
#endif
};

#ifdef GEARSVK_CEREAL
CEREAL_REGISTER_TYPE (PyPass)
CEREAL_REGISTER_POLYMORPHIC_RELATION (Pass, PyPass)
#endif

#endif