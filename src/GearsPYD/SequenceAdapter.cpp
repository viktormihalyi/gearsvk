#include "SequenceAdapter.hpp"

// from GearsVk
#include "Assert.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "StimulusAdapterView.hpp"
#include "StimulusAdapterForPresentable.hpp"
#include "Surface.hpp"
#include "VulkanEnvironment.hpp"
#include "CommandLineFlag.hpp"
#include "ImageData.hpp"
#include "Resource.hpp"

// from Gears
#include "core/Pass.h"
#include "core/Sequence.h"
#include "core/Stimulus.h"

// from std
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <map>



class RandomExporter : public IRandomExporter {
private:
    GVK::DeviceExtra& device;

    size_t randomValueLimit;
    size_t histogramBins;

    std::vector<uint32_t> values;
    std::vector<uint32_t> histogram;

public:
    RandomExporter (GVK::DeviceExtra& device, size_t randomValueLimit, size_t histogramBins)
        : device { device }
        , randomValueLimit { randomValueLimit }
        , histogramBins { histogramBins }
    {
        values.reserve (randomValueLimit);
        histogram.resize (histogramBins, 0);
    }

    virtual ~RandomExporter () override = default;

    virtual bool IsEnabled () override
    {
        return values.size () < randomValueLimit;
    }

    virtual void OnRandomTextureDrawn (GVK::RG::ImageResource& randomTexture, uint32_t resourceIndex, uint32_t frameIndex) override
    {
        GVK::ImageData imgasd (device, *randomTexture.GetImages (resourceIndex)[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        std::vector<uint32_t> as32BitUint;

        as32BitUint.resize (imgasd.width * imgasd.height * imgasd.componentByteSize / 4);
        memcpy (as32BitUint.data (), imgasd.data.data (), as32BitUint.size () * sizeof (uint32_t));

        const int64_t overshoot = values.size () + as32BitUint.size () - randomValueLimit;
        if (overshoot > 0) {
            as32BitUint.resize (as32BitUint.size () - overshoot);
        }
        
        values.insert (values.end (), as32BitUint.begin (), as32BitUint.end ());

        constexpr size_t limit = std::numeric_limits<uint32_t>::max ();
        for (uint32_t rndval : as32BitUint) {
            histogram[std::floor (static_cast<double> (rndval) / limit * histogramBins)]++;
        }

        if (!IsEnabled ()) {
            Export ();
        }
    }

    void Export ()
    {
        std::cout << "Exporting randoms..." << std::endl;

        const std::filesystem::path dir = std::filesystem::temp_directory_path () / "GearsVk" / "RandomExport";

        {
            const std::filesystem::path valuesFilePath = dir / "values.txt";
            Utils::EnsureParentFolderExists (valuesFilePath);
            std::ofstream valuesFile { valuesFilePath.string (), std::fstream::out };
            if (GVK_VERIFY (valuesFile.is_open ())) {
                for (uint32_t rndval : values) {
                    valuesFile << rndval << std::endl;
                }
            }
        }

        {
            std::ofstream histogramFile { (dir / "histogram.txt").string (), std::fstream::out };
            if (GVK_VERIFY (histogramFile.is_open ())) {
                for (size_t i = 0; i < histogram.size (); ++i) {
                    histogramFile << i << " " << histogram[i] << std::endl;
                }
            }
        }

        std::cout << "Done!" << std::endl;
    }
};


class NoRandomExporter : public IRandomExporter {
public:
    virtual ~NoRandomExporter () override = default;
    virtual bool IsEnabled () override { return false; }
    virtual void OnRandomTextureDrawn (GVK::RG::ImageResource&, uint32_t, uint32_t) override {}
};


Utils::CommandLineOnOffFlag saveRandomsFlag { "--saveRandoms", "Saves random textures to %temp%/GearsVk/" };


static std::unique_ptr<IRandomExporter> GetRandomExporterImpl (GVK::DeviceExtra& device)
{
    if (saveRandomsFlag.IsFlagOn ()) {
        return std::make_unique<RandomExporter> (device, 500'000, 20);
    }

    return std::make_unique<NoRandomExporter> ();
}



static bool IsEquivalentStimulus (const Stimulus& left, const Stimulus& right)
{
    if (left.passes.size () != right.passes.size ()) {
        return false;
    }

    for (size_t i = 0; i < left.passes.size (); ++i) {
        const Pass& leftPass  = *left.passes[i];
        const Pass& rightPass = *right.passes[i];

        if (leftPass.rasterizationMode != rightPass.rasterizationMode) {
            return false;
        }

        const std::string leftVert = leftPass.getStimulusGeneratorVertexShaderSource (leftPass.rasterizationMode);
        const std::string leftGeom = leftPass.getStimulusGeneratorGeometryShaderSource (leftPass.rasterizationMode);
        const std::string leftFrag = leftPass.getStimulusGeneratorShaderSource ();

        const std::string rightVert = rightPass.getStimulusGeneratorVertexShaderSource (rightPass.rasterizationMode);
        const std::string rightGeom = rightPass.getStimulusGeneratorGeometryShaderSource (rightPass.rasterizationMode);
        const std::string rightFrag = rightPass.getStimulusGeneratorShaderSource ();

        if (leftVert != rightVert) {
            return false;
        }
        if (leftGeom != rightGeom) {
            return false;
        }
        if (leftFrag != rightFrag) {
            return false;
        }

        if (leftPass.shaderColors != rightPass.shaderColors) {
            return false;
        }
        if (leftPass.shaderVariables != rightPass.shaderVariables) {
            return false;
        }
        if (leftPass.shaderVectors != rightPass.shaderVectors) {
            return false;
        }
    }

    return left.sequence->maxRandomGridWidth == right.sequence->maxRandomGridWidth &&
           left.sequence->maxRandomGridHeight == right.sequence->maxRandomGridHeight &&
           left.requiresClearing == right.requiresClearing &&
           left.clearColor == right.clearColor &&
           left.usesForwardRendering == right.usesForwardRendering &&
           left.randomGridWidth == right.randomGridWidth &&
           left.randomGridHeight == right.randomGridHeight &&

           left.toneMappingMode == right.toneMappingMode &&
           left.toneRangeMin == right.toneRangeMin &&
           left.toneRangeMax == right.toneRangeMax &&
           left.toneRangeMean == right.toneRangeMean &&
           left.toneRangeVar == right.toneRangeVar &&

           left.mono == right.mono &&
           left.sequence->fieldWidth_um == right.sequence->fieldWidth_um &&
           left.sequence->fieldHeight_um == right.sequence->fieldHeight_um &&
           left.doesDynamicToneMapping == right.doesDynamicToneMapping &&
           left.gammaSamplesCount == right.gammaSamplesCount &&
           memcmp (left.gamma, right.gamma, left.gammaSamplesCount) == 0 &&
           memcmp (left.temporalWeights, right.temporalWeights, 64) == 0 &&
           left.getRandomGeneratorShaderSource () == right.getRandomGeneratorShaderSource ();
}


SequenceAdapter::SequenceAdapter (GVK::VulkanEnvironment& environment, const Sequence::P& sequence)
    : sequence { sequence }
    , environment { environment }
    , timings { 0 }
    , randomExporter { GetRandomExporterImpl (*environment.deviceExtra) }
{
    StimulusAdapterViews ();

    firstFrameMs = 0;
    lastNs       = GVK::TimePoint::SinceApplicationStart ().AsMilliseconds ();
    obs.Observe (presentedFrameIndexEvent, [&] (uint32_t idx) {
        const double frameDisplayRateMs = 1.0 / 60.0 * 1000.0;

        const auto nowT  = GVK::TimePoint::SinceApplicationStart ();
        const auto nowNs = nowT.AsMilliseconds ();
        std::cout << "frame index shown: " << idx << " (ns: " << nowNs << ", delta ns: " << (nowNs - lastNs) << ", delta from expected: " << ((idx * frameDisplayRateMs) - nowNs) << ")" << std::endl;
        lastNs = nowNs;
    });
}


void SequenceAdapter::StimulusAdapterViews ()
{
    std::map<std::shared_ptr<Stimulus const>, std::shared_ptr<StimulusAdapterView>> created;

    size_t stindex = 0;

    for (auto& [_, stim] : sequence->getStimuli ()) {
        std::shared_ptr<StimulusAdapterView> equivalentAdapter;
        for (const auto& [mappedStimulus, adapter] : created) {
            if (IsEquivalentStimulus (*mappedStimulus, *stim)) {
                equivalentAdapter = adapter;
                break;
            }
        }

        if (equivalentAdapter != nullptr) {
            views[stim] = equivalentAdapter;
        } else {
            views[stim]   = std::make_unique<StimulusAdapterView> (environment, stim);
            created[stim] = views[stim];
        }

        ++stindex;
    }
}


// render finished for PREVIOUS frame
void SequenceTiming::OnImageAcquisitionReturned (uint32_t)
{
}

// render finished for PREVIOUS frame
void SequenceTiming::OnImageFenceWaitEnded (uint32_t frameIndex)
{
    const GVK::TimePoint currTime = GVK::TimePoint::SinceEpoch ();

    //std::cout << "current Time ms: " << std::fixed << currTime.AsMilliseconds () << " render  finished " << frameIndex << std::endl;

    frameRenderFinished[frameIndex] = currTime;

    timeData.resize (currentFrameIndex + 1);
    timeData[currentFrameIndex].beg = currTime;
}


// present finished for PREVIOUS frame
void SequenceTiming::OnImageAcquisitionEnded (uint32_t frameIndex)
{
    const GVK::TimePoint currTime = GVK::TimePoint::SinceEpoch ();

    //std::cout << "current Time ms: " << std::fixed << currTime.AsMilliseconds () << " present finished " << frameIndex << std::endl;

    framePresentFinished[frameIndex] = currTime;
    timeData.resize (currentFrameIndex + 1);
    timeData[currentFrameIndex].end = currTime;

    std::cout << "currentFrameIndex: "
              << currentFrameIndex
              << " - rendering finished at "
              << timeData[currentFrameIndex].end.AsSeconds ()
              << " sec with "
              << std::fixed << GVK::TimePoint (framePresentFinished[frameIndex] - frameRenderFinished[frameIndex]).AsMilliseconds ()
              << " ms accuracy"
              << std::endl;
}



void SequenceAdapter::RenderFrameIndex (const uint32_t frameIndex)
{
    if (GVK_ERROR (renderer == nullptr)) {
        return;
    }

    if (currentPresentable->GetWindow ().GetWidth () == 0 && currentPresentable->GetWindow ().GetHeight () == 0) {
        return;
    }

    if (frameIndex == 1) {
        firstFrameMs = GVK::TimePoint::SinceApplicationStart ().AsMilliseconds ();
        std::cout << "firstFrameMs = " << firstFrameMs << std::endl;
    }

    try {
        Stimulus::CP stim = sequence->getStimulusAtFrame (frameIndex);
        if (GVK_VERIFY (stim != nullptr)) {
            timings.currentFrameIndex = frameIndex;
            views[stim]->RenderFrameIndex (*renderer, currentPresentable, stim, frameIndex, timings, *randomExporter);
            lastRenderedFrameIndex = frameIndex;
        }
    } catch (GVK::OutOfDateSwapchain& ex) {
        if (currentPresentable->GetWindow ().GetWidth () == 0 && currentPresentable->GetWindow ().GetHeight () == 0) {
            return;
        }
        environment.Wait ();
        views.clear ();
        currentPresentable->GetSwapchain ().Recreate ();
        StimulusAdapterViews ();
        SetCurrentPresentable (currentPresentable);
    }
}


void SequenceAdapter::Wait ()
{
    if (GVK_VERIFY (renderer != nullptr)) {
        renderer->Wait ();
    }
}


void SequenceAdapter::SetCurrentPresentable (std::shared_ptr<GVK::Presentable> presentable)
{
    currentPresentable = presentable;

    for (auto& [stim, view] : views) {
        view->CreateForPresentable (currentPresentable);
    }

    renderer = std::make_unique<GVK::RG::SynchronizedSwapchainGraphRenderer> (*environment.deviceExtra, presentable->GetSwapchain ());
}


std::shared_ptr<GVK::Presentable> SequenceAdapter::GetCurrentPresentable ()
{
    return currentPresentable;
}


void SequenceAdapter::RenderFullOnExternalWindow ()
{
    std::unique_ptr<GVK::Presentable> presentable = std::make_unique<GVK::Presentable> (environment, std::make_unique<GVK::HiddenGLFWWindow> ());

    GVK::Window& window = presentable->GetWindow ();

    //window->SetWindowMode (GVK::Window::Mode::Fullscreen);

    GVK::SingleEventObserver kobs;
    kobs.Observe (window.events.keyPressed, [&] (uint32_t key) {
        // F11
        if (key == 300) {
            window.SetWindowMode (window.GetWindowMode () == GVK::Window::Mode::Windowed
                                      ? GVK::Window::Mode::Fullscreen
                                      : GVK::Window::Mode::Windowed);
        }
        std::cout << key << std::endl;
    });

    SetCurrentPresentable (std::move (presentable));

    window.Show ();

    uint32_t frameIndex = 1; // TODO do all sequences start at frame 1?

    window.DoEventLoop ([&] (bool& shouldStop) {
        if (window.GetWidth () == 0 && window.GetHeight () == 0) {
            return;
        }

        RenderFrameIndex (frameIndex);

        frameIndex++;

        if (frameIndex == sequence->getDuration ()) {
            shouldStop = true;
        }
    });

    window.Close ();
}