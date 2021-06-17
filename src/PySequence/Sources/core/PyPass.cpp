#include "core/PyPass.h"
#include "core/PySequence.h"
#include "core/PyStimulus.h"

#include "PyExtract.hpp"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#include "Utils/Utils.hpp"
#include "Utils/Assert.hpp"


struct PyPass::Impl {
    pybind11::object joiner;
    pybind11::object pythonObject;
};


PyPass::PyPass ()
    : impl { std::make_unique<Impl> () }
{
}


PyPass::~PyPass () = default;


pybind11::object PyPass::setJoiner (pybind11::object joiner)
{
    impl->joiner = joiner;
    return impl->joiner;
}


pybind11::object PyPass::getJoiner ()
{
    return impl->joiner;
}


pybind11::object PyPass::setPythonObject (pybind11::object o)
{
    impl->pythonObject = o;
    return impl->pythonObject;
}


pybind11::object PyPass::getPythonObject ()
{
    return impl->pythonObject;
}


void PyPass::setPolygonMask (std::string mode, pybind11::object o)
{
    // TODO
    return;
#if 0
    using namespace pybind11;

    if (mode == "fullscreen") {
        rasterizationMode = RasterizationMode::fullscreen;
    } else if (mode == "triangles") {
        list exl = o.cast<list> ();
        if (!exl.check ()) {
            PyExtract<std::string> exs (o);
            if (!exs.check ()) {
                std::stringstream ss;
                ss << "In 'triangle' mode, polygonMask must a list of float2 dicts! e.g. [{'x':1, 'y':1},{'x':0,'y':1},{'x':0,'y':0}]";
                PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
                //boost::python::throw_error_already_set ();
            }
        } else {
            list l = exl ();
            polygonMask.clear ();

            rasterizationMode = RasterizationMode::triangles;

            for (int i = 0; i < len (l); i++) {
                {
                    PyExtract<dict> pd (l[i]);
                    if (!pd.check ()) {
                        std::stringstream ss;
                        ss << "In 'triangle' mode, polygonMask must be a list of float2 dicts! e.g. [{'x':1, 'y':1},{'x':0,'y':1},{'x':0,'y':0}]";
                        PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
                        boost::python::throw_error_already_set ();
                    }
                    Gears::PythonDict d (pd);
                    polygonMask.push_back (d.asFloat3 ().xy);
                }
            }
        }
    } else if (mode == "quads") {
        PyExtract<list> exl (o);
        if (!exl.check ()) {
            PyExtract<std::string> exs (o);
            if (!exs.check ()) {
                std::stringstream ss;
                ss << "In quads mode, polygonMask must be a list of quad dicts! e.g. [{'x':1, 'y':1, 'width':50, 'height':50, 'pif':0}]";
                PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
                boost::python::throw_error_already_set ();
            }
        } else {
            list l = exl ();
            quads.clear ();

            rasterizationMode = RasterizationMode::quads;

            for (int i = 0; i < len (l); i++) {
                {
                    PyExtract<dict> pd (l[i]);
                    if (!pd.check ()) {
                        std::stringstream ss;
                        ss << "In quads mode, polygonMask must be a list of quad dicts! e.g. [{'x':1, 'y':1, 'width':50, 'height':50, 'pif':0}]";
                        PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
                        boost::python::throw_error_already_set ();
                    }
                    Gears::PythonDict d (pd);
                    QuadData          qd;
                    qd.x          = d.getFloat ("x");
                    qd.y          = d.getFloat ("y");
                    qd.halfwidth  = d.getFloat ("width") * 0.5f;
                    qd.halfheight = d.getFloat ("height") * 0.5f;
                    qd.pif        = d.getFloat ("pif");
                    qd.motion     = d.getFloat ("motion");
                    quads.push_back (qd);
                }
            }
        }
    }
#endif
}

