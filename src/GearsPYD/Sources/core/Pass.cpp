#include "stdafx.h"

#include "core/Pass.h"
#include "core/Sequence.h"
#include "core/Stimulus.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#include "Utils.hpp"
#include "Assert.hpp"


pybind11::object PyPass::setJoiner (pybind11::object joiner)
{
    this->joiner = joiner;
    return this->joiner;
}


pybind11::object PyPass::setPythonObject (pybind11::object o)
{
    pythonObject = o;
    return pythonObject;
}


pybind11::object PyPass::getPythonObject ()
{
    return pythonObject;
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
            extract<std::string> exs (o);
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
                    extract<dict> pd (l[i]);
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
        extract<list> exl (o);
        if (!exl.check ()) {
            extract<std::string> exs (o);
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
                    extract<dict> pd (l[i]);
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

