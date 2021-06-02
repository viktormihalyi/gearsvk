#include "core/Stimulus.h"

#include "core/PythonDict.h"
#include "stdafx.h"
#include "Assert.hpp"

#include "core/Pass.h"
#include "core/Sequence.h"
#include "filter/SpatialFilter.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

//#include "Eigen/Dense"
//#include "Eigen/SVD"


pybind11::object PyStimulus::setGamma (pybind11::object gammaList, bool invert)
{
    using namespace pybind11;
    list l            = extract<list> (gammaList) ();
    gammaSamplesCount = len (l);
    if (gammaSamplesCount > 101) {
        gammaSamplesCount = 101;
        PyErr_WarnEx (PyExc_UserWarning, "Only the first 101 gamma samples are used!", 2);
    }
    for (int i = 0; i < gammaSamplesCount; i++) {
        gamma[i] = extract<float> (l[i]) ();
    }

    if (invert) {
        gamma[0] = 0;
        float fiGamma[101];
        int   g = 0;
        for (int ifi = 1; ifi < gammaSamplesCount - 1; ifi++) {
            float f = (float)ifi / (gammaSamplesCount - 1);
            while (gamma[g] < f && g < gammaSamplesCount)
                g++;
            fiGamma[ifi] = ((g - 1) + (f - gamma[g - 1]) / (gamma[g] - gamma[g - 1])) / (gammaSamplesCount - 1);
        }
        fiGamma[0]                     = 0;
        fiGamma[gammaSamplesCount - 1] = 1;
        for (int u = 0; u < gammaSamplesCount; u++)
            gamma[u] = fiGamma[u];
    }
    return gammaList;
}


pybind11::object PyStimulus::setTemporalWeights (pybind11::object twList, bool fullScreen)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
#if 0
    fullScreenTemporalFiltering = fullScreen;
    using namespace boost::python;
    list l      = extract<list> (twList);
    uint length = len (l);
    if (length > 1)
        doesToneMappingInStimulusGenerator = false;
    if (length > 64) {
        PyErr_Warn (PyExc_Warning, "Temporal weights must be at most 64 values. Not setting temporal filtering weights!");
    } else {
        int u             = 0;
        temporalWeightMax = -FLT_MAX;
        temporalWeightMin = FLT_MAX;
        for (int i = 64 - length; i < 64; i++, u++) {
            temporalWeights[i] = l[u].cast<float> ();
            temporalWeightMin  = min (temporalWeightMin, temporalWeights[i]);
            temporalWeightMax  = max (temporalWeightMax, temporalWeights[i]);
        }
        for (unsigned int i = 0; i < 64 - length; i++)
            temporalWeights[i] = 0;
        temporalMemoryLength = length;
    }
#endif
    return twList;
}


//pybind11::object PyStimulus::set(pybind11::object settings)
//{
//	using namespace boost::python;
//	dict d = extract<dict>(settings);
//	Gears::PythonDict pd(d);
//	pd.process( [&](std::string key) {
//		if(		key == "duration"			)	duration = pd.getFloat(key);
//		else if(key == "shaderVariables"	) {
//			pd.forEach(key, [&](object element) {
//				dict dd = extract<dict>(element);
//				Gears::PythonDict funcDict(dd);
//				setShaderVariable( funcDict.getString("name"), funcDict.getFloat("value"));
//			});
//		}
//		else if(key == "shaderColors"	) {
//			pd.forEach(key, [&](object element) {
//				dict dd = extract<dict>(element);
//				Gears::PythonDict funcDict(dd);
//				glm::vec3 v = funcDict.getFloat3("value");
//				setShaderColor( funcDict.getString("name"), -2, v.x, v.y, v.z);
//			});
//		}
//		else if(key == "shaderFunctions"	) {
//			pd.forEach(key, [&](object element) {
//				dict dd = extract<dict>(element);
//				Gears::PythonDict funcDict(dd);
//				setShaderFunction( funcDict.getString("name"), funcDict.getString("src"));
//			});
//		}
//		else if(key == "stimulusGeneratorShaderSource"	)  stimulusGeneratorShaderSource = pd.getString(key);
//	});
//	return object();
//}


pybind11::object PyStimulus::onStart (pybind11::object callback)
{
    startCallback = callback;
    return startCallback;
}


pybind11::object PyStimulus::onFrame (pybind11::object callback)
{
    frameCallback = callback;
    return frameCallback;
}


pybind11::object PyStimulus::onFinish (pybind11::object callback)
{
    finishCallback = callback;
    return finishCallback;
}


pybind11::object PyStimulus::setJoiner (pybind11::object joiner)
{
    this->joiner = joiner;
    return this->joiner;
}


pybind11::object PyStimulus::setPythonObject (pybind11::object o)
{
    pythonObject = o;
    return pythonObject;
}


pybind11::object PyStimulus::getPythonObject () const
{
    return pythonObject;
}


pybind11::object PyStimulus::getMeasuredHistogramAsPythonList ()
{
    pybind11::list l;
    for (unsigned int i = 0; i < measuredHistogram.size (); i++)
        l.append (measuredHistogram[i]);
    return l;
}


pybind11::object PyStimulus::setMeasuredDynamicsFromPython (float            measuredToneRangeMin,
                                                          float            measuredToneRangeMax,
                                                          float            measuredMean,
                                                          float            measuredVariance,
                                                          pybind11::object histogramList) const
{
    const_cast<PyStimulus*> (this)->measuredToneRangeMin = measuredToneRangeMin;
    const_cast<PyStimulus*> (this)->measuredToneRangeMax = measuredToneRangeMax;
    const_cast<PyStimulus*> (this)->measuredMean         = measuredMean;
    const_cast<PyStimulus*> (this)->measuredVariance     = measuredVariance;
    const_cast<PyStimulus*> (this)->measuredHistogram.clear ();
    for (int i = 0; i < pybind11::len (histogramList); i++) {
        //const_cast<Stimulus*> (this)->measuredHistogram.push_back (histogramList[i].cast<float> ()));
    }
    return histogramList;
}


pybind11::object PyStimulus::setForwardRenderingCallback (pybind11::object cb)
{
    usesForwardRendering           = true;
    this->forwardRenderingCallback = cb;
    return this->forwardRenderingCallback;
}


pybind11::object PyStimulus::setTemporalWeightingFunction (std::string func, int memoryLength, bool fullscreen, float minPlot, float maxPlot)
{
    if (memoryLength > 1)
        doesToneMappingInStimulusGenerator = false;
    fullScreenTemporalFiltering = fullscreen;
    temporalMemoryLength        = memoryLength;
    temporalFilterFuncSource    = func;
    temporalWeightMin           = minPlot;
    temporalWeightMax           = maxPlot;
    return pybind11::object ();
}


void PyStimulus::registerCallback (uint msg, pybind11::object callback)
{
    for (auto& o : callbacks[msg])
        if (o == callback)
            return;
    callbacks[msg].push_back (callback);
}


pybind11::object PyStimulus::setLtiMatrix (pybind11::object mList)
{
    std::runtime_error ("DISABLED CODE");
#if 0
    fullScreenTemporalFiltering = true;
    using namespace boost::python;
    list l      = extract<list> (mList);
    uint length = len (l);
    if (length > 1)
        doesToneMappingInStimulusGenerator = false;

    if (length == 16) {
        temporalProcessingStateTransitionMatrix[0] = glm::mat4 (
            extract<float> (l[0]), extract<float> (l[1]), extract<float> (l[2]), extract<float> (l[3]),
            extract<float> (l[4]), extract<float> (l[5]), extract<float> (l[6]), extract<float> (l[7]),
            extract<float> (l[8]), extract<float> (l[9]), extract<float> (l[10]), extract<float> (l[11]),
            extract<float> (l[12]), extract<float> (l[13]), extract<float> (l[14]), extract<float> (l[15]));

        temporalProcessingStateCount = 3;
        finishLtiSettings ();
    } else if (length = 64) {
        temporalProcessingStateTransitionMatrix[0] = glm::mat4 (
            extract<float> (l[0]), extract<float> (l[1]), extract<float> (l[2]), extract<float> (l[3]),
            extract<float> (l[8]), extract<float> (l[9]), extract<float> (l[10]), extract<float> (l[11]),
            extract<float> (l[16]), extract<float> (l[17]), extract<float> (l[18]), extract<float> (l[19]),
            extract<float> (l[24]), extract<float> (l[25]), extract<float> (l[26]), extract<float> (l[27]));
        temporalProcessingStateTransitionMatrix[1] = glm::mat4 (
            extract<float> (l[4]), extract<float> (l[5]), extract<float> (l[6]), extract<float> (l[7]),
            extract<float> (l[12]), extract<float> (l[13]), extract<float> (l[14]), extract<float> (l[15]),
            extract<float> (l[20]), extract<float> (l[21]), extract<float> (l[22]), extract<float> (l[23]),
            extract<float> (l[28]), extract<float> (l[29]), extract<float> (l[30]), extract<float> (l[31]));
        temporalProcessingStateTransitionMatrix[2] = glm::mat4 (
            extract<float> (l[32]), extract<float> (l[33]), extract<float> (l[34]), extract<float> (l[35]),
            extract<float> (l[40]), extract<float> (l[41]), extract<float> (l[42]), extract<float> (l[43]),
            extract<float> (l[48]), extract<float> (l[49]), extract<float> (l[50]), extract<float> (l[51]),
            extract<float> (l[56]), extract<float> (l[57]), extract<float> (l[58]), extract<float> (l[59]));
        temporalProcessingStateTransitionMatrix[3] = glm::mat4 (
            extract<float> (l[36]), extract<float> (l[37]), extract<float> (l[38]), extract<float> (l[39]),
            extract<float> (l[44]), extract<float> (l[45]), extract<float> (l[46]), extract<float> (l[47]),
            extract<float> (l[52]), extract<float> (l[53]), extract<float> (l[54]), extract<float> (l[55]),
            extract<float> (l[60]), extract<float> (l[61]), extract<float> (l[62]), extract<float> (l[63]));

        temporalProcessingStateCount = 7;
        finishLtiSettings ();

    } else {
        PyErr_SetString (PyExc_TypeError, "Temporal processing by a Linear Time Invariant system requires a 4x4 (3 states) or 8x8 (7 states) state transition matrix.");
        boost::python::throw_error_already_set ();
    }
#endif
    return mList;
}


pybind11::object PyStimulus::setLtiImpulseResponse (pybind11::object mList, uint nStates)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
#if 0
    fullScreenTemporalFiltering        = true;
    doesToneMappingInStimulusGenerator = false;

    if (nStates > 7) {
        PyErr_SetString (PyExc_TypeError, "For temporal processing by a Linear Time Invariant system the maximum number of states supported is 7.");
        boost::python::throw_error_already_set ();
    }
    if (nStates > 3)
        nStates = 7;
    else
        nStates = 3;
    using namespace boost::python;
    list l = extract<list> (mList);

    using Eigen::JacobiSVD;
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    int      nOutputs = len (l);
    VectorXd g (nOutputs);
    for (int i = 0; i < nOutputs; i++)
        g (i) = extract<float> (l[i]);

    MatrixXd h (nOutputs / 2, nOutputs / 2);
    MatrixXd hshift (nOutputs / 2, nOutputs / 2);

    for (int i = 0; i < nOutputs / 2; i++)
        for (int j = 0; j < nOutputs / 2; j++) {
            h (i, j)      = g (i + j);
            hshift (i, j) = g (i + j + 1);
        }

    // decompose Hankel matrix h
    JacobiSVD<MatrixXd> svdh (h, Eigen::DecompositionOptions::ComputeFullU | Eigen::DecompositionOptions::ComputeFullV);

    VectorXd ss (svdh.singularValues ().size ());
    for (int i = 0; i < svdh.singularValues ().size (); i++)
        ss (i) = sqrt (svdh.singularValues () (i));
    MatrixXd ho = svdh.matrixU () * ss.asDiagonal ();
    MatrixXd hc = ss.asDiagonal () * svdh.matrixV ().transpose ();

    // compute pseudo inverses of ho and hc
    JacobiSVD<MatrixXd> svdho (ho, Eigen::DecompositionOptions::ComputeFullU | Eigen::DecompositionOptions::ComputeFullV);
    JacobiSVD<MatrixXd> svdhc (hc, Eigen::DecompositionOptions::ComputeFullU | Eigen::DecompositionOptions::ComputeFullV);
    MatrixXd            hopi (nOutputs / 2, nOutputs / 2);
    svdho.pinv (hopi);
    MatrixXd hcpi (nOutputs / 2, nOutputs / 2);
    svdhc.pinv (hcpi);

    // assemble state matrix
    MatrixXd a = hopi.topLeftCorner (nStates, nOutputs / 2) * hshift * hcpi.topLeftCorner (nOutputs / 2, nStates);
    MatrixXd b = hc.topLeftCorner (nStates, 1);
    MatrixXd c = ho.topLeftCorner (1, nStates);
    MatrixXd d (1, 1);
    d (0, 0) = g (0);
    MatrixXd p (nStates + 1, nStates + 1);

    c = c * a;

    p << d, c, b, a;

    VectorXd state         = VectorXd::Zero (nStates + 1);
    state (0)              = 1;
    double errorSum        = 0;
    double squaredErrorSum = 0;
    for (int i = 0; i < nOutputs; i++) {
        state      = p * state;
        double err = state (0) - g (i);
        errorSum += err;
        squaredErrorSum += err * err;
        state (0) = 0;
    }
    squaredErrorSum /= nOutputs;

    std::stringstream ssbb;
    ssbb << "Optimal LTI system found. Total error of fitting: " << errorSum << ", MSE: " << squaredErrorSum << "." << std::endl;
    PyErr_Warn (PyExc_Warning, ssbb.str ().c_str ());
#pragma warning(push)
#pragma warning(disable : 4244)
    temporalProcessingStateCount = nStates;
    if (nStates == 3) {
        temporalProcessingStateTransitionMatrix[0] = glm::mat4 (
            p (0, 0), p (0, 1), p (0, 2), p (0, 3),
            p (1, 0), p (1, 1), p (1, 2), p (1, 3),
            p (2, 0), p (2, 1), p (2, 2), p (2, 3),
            p (3, 0), p (3, 1), p (3, 2), p (3, 3));
    } else {
        temporalProcessingStateTransitionMatrix[0] = glm::mat4 (
            p (0, 0), p (0, 1), p (0, 2), p (0, 3),
            p (1, 0), p (1, 1), p (1, 2), p (1, 3),
            p (2, 0), p (2, 1), p (2, 2), p (2, 3),
            p (3, 0), p (3, 1), p (3, 2), p (3, 3));
        temporalProcessingStateTransitionMatrix[1] = glm::mat4 (
            p (0, 4), p (0, 5), p (0, 6), p (0, 7),
            p (1, 4), p (1, 5), p (1, 6), p (1, 7),
            p (2, 4), p (2, 5), p (2, 6), p (2, 7),
            p (3, 4), p (3, 5), p (3, 6), p (3, 7));
        temporalProcessingStateTransitionMatrix[2] = glm::mat4 (
            p (4, 0), p (4, 1), p (4, 2), p (4, 3),
            p (5, 0), p (5, 1), p (5, 2), p (5, 3),
            p (6, 0), p (6, 1), p (6, 2), p (6, 3),
            p (7, 0), p (7, 1), p (7, 2), p (7, 3));
        temporalProcessingStateTransitionMatrix[3] = glm::mat4 (
            p (4, 4), p (4, 5), p (4, 6), p (4, 7),
            p (5, 4), p (5, 5), p (5, 6), p (5, 7),
            p (6, 4), p (6, 5), p (6, 6), p (6, 7),
            p (7, 4), p (7, 5), p (7, 6), p (7, 7));
    }
#pragma warning(pop)
    finishLtiSettings ();
#endif
    return mList;
}


void PyStimulus::OnPassAdded (std::shared_ptr<Pass> pass)
{
    PyPass* pyPass = dynamic_cast<PyPass*> (pass.get ());
    if (GVK_VERIFY (pyPass != nullptr)) {
        pyPass->joiner ();
    }
}
