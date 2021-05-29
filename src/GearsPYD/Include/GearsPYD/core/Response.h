#pragma once

#include "stdafx.h"
#include <memory>

#include <algorithm>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class Sequence;
class SpatialFilter;

//! A structure that contains all stimulus parameters.
class Response : public std::enable_shared_from_this<Response> {
public:
    std::string question;
    bool        loop;

    struct Button {
        std::string label;
        float       xcoord, ycoord, width, height;
        uint        key;
        bool        visible;
    };
    std::vector<Button> buttons;
    unsigned int        duration; //frames
    unsigned int        startingFrame;

    std::shared_ptr<Sequence> sequence; //< Part of this sequence.


    Response ();
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (Response);

    pybind11::object pythonObject;
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject () const;

    pybind11::object joiner;
    pybind11::object setJoiner (pybind11::object joiner);


    std::map<uint, std::vector<pybind11::object>> callbacks;
    void                                          registerCallback (uint msg, pybind11::object callback);

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

    void addButton (std::string label, float x, float y, float w, float h, uint key, bool visible);

    void setSequence (std::shared_ptr<Sequence> sequence)
    {
        this->sequence = sequence;
    }

    std::shared_ptr<Sequence> getSequence () { return sequence; }
};
