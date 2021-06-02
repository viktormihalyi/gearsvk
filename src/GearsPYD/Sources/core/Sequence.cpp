#include "core/Sequence.h"
#include "PythonDict.h"
#include "core/Response.h"
#include "core/Stimulus.h"
#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <sstream>


pybind11::object PySequence::set (pybind11::object settings)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
#if 0
	using namespace boost::python;
	dict d = extract<dict>(settings);
	Gears::PythonDict pd(d);
	pd.process( [&](std::string key) {
		if(key == "geometry") {
			Gears::PythonDict geomSettings = pd.getSubDict(key);
			geomSettings.process( [&](std::string key) {
				if(		key == "fieldWidth_um")		fieldWidth_um	= geomSettings.getFloat(key);
				else if(key == "fieldHeight_um")	fieldHeight_um	= geomSettings.getFloat(key);
				else if(key == "fieldWidth_px")		fieldWidth_px	= geomSettings.getUint(key);
				else if(key == "fieldHeight_px")	fieldHeight_px	= geomSettings.getUint(key);
				else if(key == "fieldLeft_px")		fieldLeft_px	= geomSettings.getUint(key);
				else if(key == "fieldBottom_px")	fieldBottom_px	= geomSettings.getUint(key);
			});
		}
	});
#endif
    return settings;
}


PySequence::P PySequence::setAgenda (pybind11::object agenda)
{
    using namespace pybind11;
    list                     l = agenda.cast<list> ();
    std::vector<Stimulus::P> stimsUnderConstruction;
    for (int i = 0; i < len (l); i++) {
        {
            extract<Response::P> s (l[i]);
            if (s.check ()) {
                addResponse (s ());
                continue;
            }
        }
        {
            extract<Stimulus::P> s (l[i]);
            if (s.check ()) {
                Stimulus::P sp = s ();
                stimsUnderConstruction.push_back (sp);
                addStimulus (sp);
                continue;
            }
        }
        {
            extract<RaiseSignal::P> s (l[i]);
            if (s.check ()) {
                RaiseSignal::P p = s ();
                raiseSignal (p->getChannel ());
                continue;
            }
        }
        {
            extract<ClearSignal::P> s (l[i]);
            if (s.check ()) {
                ClearSignal::P p = s ();
                clearSignal (p->getChannel ());
                continue;
            }
        }
        {
            extract<RaiseAndClearSignal::P> s (l[i]);
            if (s.check ()) {
                RaiseAndClearSignal::P p = s ();
                raiseAndClearSignal (p->getChannel (), p->getHoldFrameCount ());
                continue;
            }
        }
        {
            extract<StartMeasurement::P> s (l[i]);
            if (s.check ()) {
                StartMeasurement::P p = s ();
                raiseSignal (p->getExpSyncChannel ());
                if (!setMeasurementStart ()) {
                    std::stringstream ss;
                    ss << "Item #" << i + 1 << " on agenda is a StartMeasurement, but the measurement has already been started.";
                    THROW_LOC ();
                    //PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
                    //boost::python::throw_error_already_set ();
                }
                continue;
            }
        }
        {
            extract<EndMeasurement::P> s (l[i]);
            if (s.check ()) {
                EndMeasurement::P p = s ();
                clearSignal (p->getExpSyncChannel ());
                raiseAndClearSignal (p->getMeasurementStopChannel (), p->getHoldStopFrameCount ());
                if (!setMeasurementEnd ()) {
                    std::stringstream ss;
                    ss << "Item #" << i + 1 << " on agenda is an EndMeasurement, but the measurement has already been ended.";
                    THROW_LOC ();
                    //PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
                    //boost::python::throw_error_already_set ();
                }
                continue;
            }
        }
        std::stringstream ss;
        ss << "Item #" << i + 1 << " on agenda is of unknown type.";
        //THROW_LOC ();
        //PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
        //boost::python::throw_error_already_set ();
    }
    {
        for (auto suc : stimsUnderConstruction)
            suc->onSequenceComplete ();
    }

    return getSharedPtr ();
}


pybind11::object PySequence::onReset (pybind11::object cb)
{
    resetCallback = cb;
    return resetCallback;
}


pybind11::object PySequence::setPythonObject (pybind11::object o)
{
    pythonObject = o;
    return pythonObject;
}


pybind11::object PySequence::getPythonObject ()
{
    return pythonObject;
}
