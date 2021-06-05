#include "core/PySequence.h"
#include "PythonDict.h"
#include "core/PyResponse.h"
#include "core/PyStimulus.h"
#include "PyExtract.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

#include "Utils/Assert.hpp"

#include "Utils/SourceLocation.hpp"

#define THROW_LOC() \
    throw ::std::runtime_error (::Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ())


struct PySequence::Impl {
    pybind11::object resetCallback;
    pybind11::object pythonObject;
};


PySequence::PySequence (std::string name)
    : Sequence { name }
    , impl { std::make_unique<Impl> () }
{
}


pybind11::object PySequence::set (pybind11::object settings)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
#if 0
	using namespace boost::python;
	dict d = PyExtract<dict>(settings);
	std::shared_ptr<GearsythonDict> pd(d);
	pd.process( [&](std::string key) {
		if(key == "geometry") {
			std::shared_ptr<GearsythonDict> geomSettings = pd.getSubDict(key);
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


std::shared_ptr<PySequence> PySequence::setAgenda (pybind11::object agenda)
{
    using namespace pybind11;
    list                     l = agenda.cast<list> ();
    std::vector<std::shared_ptr<PyStimulus>> stimsUnderConstruction;
    for (int i = 0; i < len (l); i++) {
        {
            PyExtract<std::shared_ptr<PyResponse>> s (l[i]);
            if (s.check ()) {
                addResponse (s ());
                continue;
            }
        }
        {
            PyExtract<std::shared_ptr<PyStimulus>> s (l[i]);
            if (s.check ()) {
                std::shared_ptr<PyStimulus> sp = s ();
                stimsUnderConstruction.push_back (sp);
                addStimulus (sp);
                continue;
            }
        }
        {
            PyExtract<std::shared_ptr<RaiseSignal>> s (l[i]);
            if (s.check ()) {
                std::shared_ptr<RaiseSignal> p = s ();
                raiseSignal (p->getChannel ());
                continue;
            }
        }
        {
            PyExtract<std::shared_ptr<ClearSignal>> s (l[i]);
            if (s.check ()) {
                std::shared_ptr<ClearSignal> p = s ();
                clearSignal (p->getChannel ());
                continue;
            }
        }
        {
            PyExtract<std::shared_ptr<RaiseAndClearSignal>> s (l[i]);
            if (s.check ()) {
                std::shared_ptr<RaiseAndClearSignal> p = s ();
                raiseAndClearSignal (p->getChannel (), p->getHoldFrameCount ());
                continue;
            }
        }
        {
            PyExtract<std::shared_ptr<StartMeasurement>> s (l[i]);
            if (s.check ()) {
                std::shared_ptr<StartMeasurement> p = s ();
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
            PyExtract<std::shared_ptr<EndMeasurement>> s (l[i]);
            if (s.check ()) {
                std::shared_ptr<EndMeasurement> p = s ();
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

    return std::dynamic_pointer_cast<PySequence> (shared_from_this ());
}


pybind11::object PySequence::onReset (pybind11::object cb)
{
    impl->resetCallback = cb;
    return impl->resetCallback;
}


pybind11::object PySequence::setPythonObject (pybind11::object o)
{
    impl->pythonObject = o;
    return impl->pythonObject;
}


pybind11::object PySequence::getPythonObject ()
{
    return impl->pythonObject;
}


void PySequence::OnStimulusAdded (std::shared_ptr<Stimulus> stimulus)
{
    PyStimulus* pyStim = dynamic_cast<PyStimulus*> (stimulus.get ());
    if (GVK_VERIFY (pyStim != nullptr)) {
        pyStim->getJoiner () ();
    }
}
