
#include "core/Sequence.h"
#include "core/SequenceRenderer.h"
#include "core/filter/KernelManager.h"

#include "gpu/Framebuffer.hpp"
#include "gpu/Pointgrid.hpp"
#include "gpu/Quad.hpp"
#include "gpu/RandomSequenceBuffer.hpp"
#include "gpu/Shader.hpp"
#include "gpu/StimulusGrid.hpp"
#include "gpu/Texture.hpp"
#include "gpu/TextureQueue.hpp"

#include "core/ShaderManager.h"
#include "core/filter/SpatialFilter.h"
#include "core/filter/fft/glFFT.h"
#include "core/filter/fft/openCLFFT.h"


#include <sstream>

KernelManager::KernelManager (SequenceRenderer::P sequenceRenderer, ShaderManager::P shaderManager)
    : sequenceRenderer (sequenceRenderer), shaderManager (shaderManager)
{
}

uint32_t KernelManager::getKernel (SpatialFilter::CP spatialFilter)
{
    std::string         slongid = spatialFilter->getKernelGeneratorShaderSourceWithParameters ();
    KernelMap::iterator i       = kernels.find (slongid);
    if (i != kernels.end ()) {
        if (i->second.fft)
            return i->second.fft->get_fullTex ();
        if (i->second.buff)
            return i->second.buff->getColorBuffer (0);
    }

    std::string fragmentShaderSource = spatialFilter->getKernelGeneratorShaderSource ();

    Shader*      kernelShader = shaderManager->loadShader (fragmentShaderSource);
    std::shared_ptr<const Sequence> sequence     = sequenceRenderer->getSequence ();

    if (spatialFilter->useFft) {
#if 0
        FFT* fft;
        if (sequenceRenderer->clFFT ())
            fft = new OPENCLFFT (sequence->fftWidth_px, sequence->fftHeight_px);
        else
            fft = new GLFFT (sequence->fftWidth_px, sequence->fftHeight_px);
        kernels[slongid] = Kernel {fft, nullptr, kernelShader};
#endif
    } else {
        FramebufferGL* bufi = new FramebufferGL (spatialFilter->horizontalSampleCount, spatialFilter->verticalSampleCount, 1, GL_RGBA16F);
        kernels[slongid]    = Kernel { nullptr, bufi, kernelShader };
    }

    return update (spatialFilter);
}

#if 0
bool KernelManager::getKernelChannels (SpatialFilter::CP spatialFilter, cl_mem& r)
{
    std::string         slongid = spatialFilter->getKernelGeneratorShaderSourceWithParameters ();
    KernelMap::iterator i       = kernels.find (slongid);
    if (i != kernels.end ()) {
        if (i->second.fft) {
            static_cast<OPENCLFFT*> (i->second.fft)->get_channels (r);
            return true;
        }
        return false;
    }
    return false;
}
#endif

uint32_t KernelManager::update (SpatialFilter::CP spatialFilter)
{
#if 0
    if (spatialFilter == nullptr) //TODO this is a bit of a hack. update will be called when the component's update method executes initially. Then the current stimulus may be anything and may not have a spatial filter.
        return 0;
    std::string         slongid = spatialFilter->getKernelGeneratorShaderSourceWithParameters ();
    KernelMap::iterator i       = kernels.find (slongid);
    if (i == kernels.end ()) {
        std::stringstream ss;
        ss << "Kernel not found. There must be some problem with kernel identification for dynamic or shared kernels.";
        throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
        //PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
        //boost::python::throw_error_already_set ();
    }
    Shader* kernelShader = i->second.kernelShader;

    std::shared_ptr<const Sequence> sequence = sequenceRenderer->getSequence ();

    auto renderKernelLambda =
        [&] (int) {
            kernelShader->enable ();
            kernelShader->bindUniformBool ("kernelGivenInFrequencyDomain", spatialFilter->kernelGivenInFrequencyDomain);
            if (spatialFilter->useFft) {
                if (spatialFilter->kernelGivenInFrequencyDomain) {
                    kernelShader->bindUniformFloat2 ("patternSizeOnRetina",
                                                     sequence->fftWidth_px / sequence->getSpatialFilteredFieldWidth_um (),
                                                     sequence->fftHeight_px / sequence->getSpatialFilteredFieldHeight_um ()
                                                     //sequence->fftWidth_px / sequence->fieldWidth_um,
                                                     //sequence->fftHeight_px / sequence->fieldHeight_um
                                                     //							512 * 512 * 1.4 * 3.1415926535897932384626433832795 / sequence->getSpatialFilteredFieldWidth_um(),
                                                     //							512 * 512 * 1.4 * 3.1415926535897932384626433832795 / sequence->getSpatialFilteredFieldHeight_um()
                    );
                    kernelShader->bindUniformFloat2 ("texelSize_um",
                                                     (float)(sequence->fftWidth_px / 3.1415926535897932384626433832795 / sequence->getSpatialFilteredFieldWidth_um () / 2000000),
                                                     (float)(sequence->fftHeight_px / 3.1415926535897932384626433832795 / sequence->getSpatialFilteredFieldHeight_um () / 200000));
                } else {
                    kernelShader->bindUniformFloat2 ("patternSizeOnRetina", sequence->getSpatialFilteredFieldWidth_um (), sequence->getSpatialFilteredFieldHeight_um ());
                    //kernelShader->bindUniformFloat2("texelSize_um", sequence->getSpatialFilteredFieldWidth_um() / spatialFilter->horizontalSampleCount, sequence->getSpatialFilteredFieldHeight_um() / spatialFilter->verticalSampleCount);
                    kernelShader->bindUniformFloat2 ("texelSize_um",
                                                     0, 0
                                                     //						sequence->getSpatialFilteredFieldWidth_um() / sequence->fftWidth_px,
                                                     //						sequence->getSpatialFilteredFieldHeight_um() / sequence->fftHeight_px
                    );
                }
            } else {
                kernelShader->bindUniformFloat2 ("patternSizeOnRetina", spatialFilter->width_um, spatialFilter->height_um);
                kernelShader->bindUniformFloat2 ("texelSize_um", spatialFilter->width_um / spatialFilter->horizontalSampleCount, spatialFilter->height_um / spatialFilter->verticalSampleCount);
            }

            for (auto& setting : spatialFilter->shaderVariables) {
                kernelShader->bindUniformFloat (setting.first.c_str (), setting.second);
            }
            for (auto& setting : spatialFilter->shaderColors) {
                kernelShader->bindUniformFloat3 (setting.first.c_str (), setting.second.x, setting.second.y, setting.second.z);
            }
            for (auto& setting : spatialFilter->shaderVectors) {
                kernelShader->bindUniformFloat2 (setting.first.c_str (), setting.second.x, setting.second.y);
            }

            sequenceRenderer->getNothing ()->renderQuad ();
            kernelShader->disable ();
        };


    if (spatialFilter->useFft) {
        FFT* fft = i->second.fft;
        fft->set_input (renderKernelLambda);
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        glOrtho (-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
        fft->redraw_input ();
        if (!spatialFilter->kernelGivenInFrequencyDomain)
            fft->do_fft ();
        return fft->get_fullTex ();
    } else {
        FramebufferGL* bufi = i->second.buff;
        bufi->setRenderTarget (0);
        renderKernelLambda (0);
        bufi->disableRenderTarget ();
        return bufi->getColorBuffer (0);
    }
#endif
    return 0;
}

void KernelManager::clear ()
{
    for (auto k : kernels) {
        if (k.second.fft)
            delete k.second.fft;
        if (k.second.buff)
            delete k.second.buff;
        // shaderManager deltes kernelShader
    }
    kernels.clear ();
}