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

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>

class Pass;
class Sequence;

class PYSEQUENCE_API PyStimulus : public Stimulus {
private:

    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    PyStimulus ();
    virtual ~PyStimulus ();

    pybind11::object setJoiner (pybind11::object joiner);
    pybind11::object getJoiner ();

    pybind11::object setForwardRenderingCallback (pybind11::object cb);

    pybind11::object set (pybind11::object settings);

    pybind11::object setGamma (pybind11::object gammaList, bool invert = false);
    pybind11::object setTemporalWeights (pybind11::object twList, bool fullScreen);
    pybind11::object setTemporalWeightingFunction (std::string func, int memoryLength, bool fullscreen, float minPlot, float maxPlot);
    pybind11::object setLtiMatrix (pybind11::object mList);
    pybind11::object setLtiImpulseResponse (pybind11::object mList, uint32_t nStates);


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

    pybind11::object onStart (pybind11::object callback);
    pybind11::object onFrame (pybind11::object callback);
    pybind11::object onFinish (pybind11::object callback);

    pybind11::object setPythonObject (pybind11::object o);
    pybind11::object getPythonObject () const;

    pybind11::object getMeasuredHistogramAsPythonList ();

    pybind11::object setMeasuredDynamicsFromPython (float            measuredToneRangeMin,
                                                    float            measuredToneRangeMax,
                                                    float            measuredMean,
                                                    float            measuredVariance,
                                                    pybind11::object histogramList) const;

    virtual void OnPassAdded (std::shared_ptr<Pass> pass) override;

    template<typename Archive>
    void serialize (Archive& ar)
    {
        ar (cereal::base_class<Stimulus> (this));
    }
};

CEREAL_REGISTER_TYPE (PyStimulus)
CEREAL_REGISTER_POLYMORPHIC_RELATION (Stimulus, PyStimulus)

#endif