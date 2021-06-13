#ifndef PYSTIMULUS_HPP
#define PYSTIMULUS_HPP

#include "PySequence/PySequenceAPI.hpp"

#include "Sequence/Stimulus.h"

#include <memory>

#include <algorithm>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "pybind11/pybind11.h"

class Pass;
class Sequence;

class PYSEQUENCE_API PyStimulus : public Stimulus {
public:
    using Stimulus::Stimulus;

    pybind11::object joiner;
    pybind11::object setJoiner (pybind11::object joiner);

    pybind11::object forwardRenderingCallback;
    pybind11::object setForwardRenderingCallback (pybind11::object cb);

    pybind11::object set (pybind11::object settings);

    pybind11::object setGamma (pybind11::object gammaList, bool invert = false);
    pybind11::object setTemporalWeights (pybind11::object twList, bool fullScreen);
    pybind11::object setTemporalWeightingFunction (std::string func, int memoryLength, bool fullscreen, float minPlot, float maxPlot);
    pybind11::object setLtiMatrix (pybind11::object mList);
    pybind11::object setLtiImpulseResponse (pybind11::object mList, uint32_t nStates);

    std::map<uint32_t, std::vector<pybind11::object>> callbacks;

    void registerCallback (uint32_t msg, pybind11::object callback);

#if 0
    template<typename T>
    bool executeCallbacks (typename std::shared_ptr<T> wevent) const
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

    pybind11::object startCallback;
    pybind11::object frameCallback;
    pybind11::object finishCallback;
    pybind11::object onStart (pybind11::object callback);
    pybind11::object onFrame (pybind11::object callback);
    pybind11::object onFinish (pybind11::object callback);

    pybind11::object pythonObject;
    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject () const;

    pybind11::object getMeasuredHistogramAsPythonList ();

    pybind11::object setMeasuredDynamicsFromPython (float            measuredToneRangeMin,
                                                    float            measuredToneRangeMax,
                                                    float            measuredMean,
                                                    float            measuredVariance,
                                                    pybind11::object histogramList) const;

    virtual void OnPassAdded (std::shared_ptr<Pass> pass) override;
};

#endif