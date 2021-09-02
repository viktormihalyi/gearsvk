#ifndef PYRESPONSE_HPP
#define PYRESPONSE_HPP

#include "Sequence/SequenceAPI.hpp"

#include "Sequence/Response.h"

#include <memory>

#include "pybind11/pybind11.h"

#ifdef GEARSVK_CEREAL
#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#endif

class Sequence;
class SpatialFilter;

//! A structure that contains all stimulus parameters.
class SEQUENCE_API PyResponse : public Response {
private:

    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    PyResponse ();
    virtual ~PyResponse ();
    
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject () const;

    pybind11::object setJoiner (pybind11::object joiner);

    void registerCallback (uint32_t msg, pybind11::object callback);

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

#ifdef GEARSVK_CEREAL
    template<typename Archive>
    void serialize (Archive& ar)
    {
        ar (cereal::base_class<Response> (this));
    }
#endif
};

#ifdef GEARSVK_CEREAL
CEREAL_REGISTER_TYPE (PyResponse)
CEREAL_REGISTER_POLYMORPHIC_RELATION (Response, PyResponse)
#endif

#endif
