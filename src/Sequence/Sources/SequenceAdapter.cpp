#include "SequenceAdapter.hpp"

// from RenderGraph
#include "Utils/Assert.hpp"
#include "Utils/CommandLineFlag.hpp"

#include "StimulusAdapter.hpp"
#include "StimulusAdapterView.hpp"

#include "RenderGraph/Window/GLFWWindow.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "VulkanWrapper/Surface.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "VulkanWrapper/Utils/ImageData.hpp"
#include "RenderGraph/Resource.hpp"

// from Gears
#include "Pass.h"
#include "Sequence.h"
#include "Stimulus.h"

// from std
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <map>

#include "spdlog/spdlog.h"


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
        GVK::ImageData randomImage (device, *randomTexture.GetImages (resourceIndex)[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        std::vector<uint32_t> as32BitUint;
        as32BitUint.resize (randomImage.width * randomImage.height * randomImage.componentByteSize / 4);
        memcpy (as32BitUint.data (), randomImage.data.data (), as32BitUint.size () * sizeof (uint32_t));

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


Utils::CommandLineOnOffFlag printSignalsFlag { "--printSignals", "Prints signals to stdout." };

SequenceAdapter::SequenceAdapter (GVK::VulkanEnvironment& environment, const std::shared_ptr<Sequence>& sequence, const std::string& sequenceNameInTitle)
    : sequence { sequence }
    , environment { environment }
    , randomExporter { GetRandomExporterImpl (*environment.deviceExtra) }
    , sequenceNameInTitle { sequenceNameInTitle }
{
    CreateStimulusAdapterViews ();

    if (printSignalsFlag.IsFlagOn ()) {
        std::cout << "======= Sequence signals =======" << std::endl;
        for (auto& signal : sequence->getSignals ()) {
            const size_t      frameIndex  = signal.first;
            const bool        clear       = signal.second.clear;
            const std::string channelName = signal.second.channel;
            const std::string channelPort = sequence->getChannels ().find (channelName)->second.portName;
            std::cout << "Signal at frame index " << frameIndex << " on channel: " << channelName << ", port: " << channelPort << ", clear: " << std::boolalpha << clear << std::endl; 
        }
        for (auto& stimulus : sequence->getStimuli ()) {
            std::cout << "======= Stimulus signals - \"" << stimulus.second->name << "\" (starting frame: " << stimulus.second->getStartingFrame () << ", duration: " << stimulus.second->getDuration () << ") =======" << std::endl;
            for (auto& signal : stimulus.second->getSignals ()) {
                const size_t      frameIndex  = signal.first;
                const bool        clear       = signal.second.clear;
                const std::string channelName = signal.second.channel;
                const std::string channelPort = sequence->getChannels ().find (channelName)->second.portName;
                std::cout << "Signal at frame index " << frameIndex << " (global: " << frameIndex + stimulus.second->getStartingFrame () << ") on channel: \"" << channelName << "\", port: \"" << channelPort << "\", clear: " << std::boolalpha << clear << std::endl; 
            }
        }
        std::cout << "======= Signals end =======" << std::endl;
    }
}


void SequenceAdapter::CreateStimulusAdapterViews ()
{
    std::map<std::shared_ptr<Stimulus const>, std::shared_ptr<StimulusAdapterView>> created;

    size_t stindex = 0;

    for (auto& [_, stim] : sequence->getStimuli ()) {
        std::shared_ptr<StimulusAdapterView> equivalentAdapter;
        for (const auto& [mappedStimulus, adapter] : created) {
            if (mappedStimulus->IsEquivalent (*stim)) {
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


static int32_t PositiveModulo (int32_t i, int32_t n)
{
    return (i % n + n) % n;
}


static void SignalImplCout (std::string_view type, std::string_view channelName, std::string_view channelPort, bool clear)
{
    std::cout << "[" << type << "] Channel: " << channelName << " (" << channelPort << "), clear: " << std::boolalpha << clear << std::endl;
}


static void SignalImpl (std::string_view type, std::string_view channelName, std::string_view channelPort, bool clear)
{
    spdlog::info ("[{}] Channel: {} ({}), clear: {}", type, channelName, channelPort, clear);
}


// previous resource index finished preseting
void SequenceAdapter::OnImageAcquisitionFenceSignaled (uint32_t resourceIndex)
{
    const uint32_t previousResourceIndex = PositiveModulo (static_cast<int32_t> (resourceIndex) - 1, renderer->GetFramesInFlight ());
    
    const size_t finishedFrameIndex = resourceIndexToRenderedFrameMapping[previousResourceIndex];

    resourceIndexToRenderedFrameMapping[previousResourceIndex] = 0;
    
    // TODO check if signal's frame index and finishedFrameIndex are the same

    const std::shared_ptr<Stimulus const> stimulus = sequence->getStimulusAtFrame (finishedFrameIndex);
    GVK_ASSERT (stimulus != nullptr);

    GVK_ASSERT (finishedFrameIndex + 1 /* TODO why +1 */ >= stimulus->getStartingFrame ());
    const size_t stimulusFrameIndex = finishedFrameIndex - stimulus->getStartingFrame ();

    {
        std::stringstream ss;
        ss << "GearsVk - " << sequenceNameInTitle;
        ss << " [stimulus: frame " << stimulusFrameIndex << " / " << stimulus->getDuration () << " (" << std::floor (static_cast<double> (stimulusFrameIndex) / stimulus->getDuration () * 100.0) << "%) ";
        ss << ", sequence: frame " << finishedFrameIndex << " / " << sequence->getDuration () << " (" << std::floor (static_cast<double> (finishedFrameIndex) / sequence->getDuration () * 100.0) << "%)] ";
        currentPresentable->GetWindow ().SetTitle (ss.str ());
    }

    auto sequenceSignals = sequence->getSignals ().equal_range (finishedFrameIndex);
    for (auto it = sequenceSignals.first; it != sequenceSignals.second; ++it) {
        auto channel = sequence->getChannels ().find (it->second.channel);
        if (GVK_VERIFY (channel != sequence->getChannels ().end ())) {
            SignalImpl ("SEQUENCE SIGNAL", it->second.channel, channel->second.portName, it->second.clear);
        }
    }
    
    auto stimulusSignals = stimulus->getSignals ().equal_range (stimulusFrameIndex);
    for (auto it = stimulusSignals.first; it != stimulusSignals.second; ++it) {
        auto channel = sequence->getChannels ().find (it->second.channel);
        if (GVK_VERIFY (channel != sequence->getChannels ().end ())) {
            SignalImpl ("STIMULUS TICK SIGNAL", it->second.channel, channel->second.portName, it->second.clear);
        }
    }
}


void SequenceAdapter::RenderFrameIndex (const uint32_t frameIndex)
{
    if (GVK_ERROR (renderer == nullptr)) {
        return;
    }

    if (currentPresentable->GetWindow ().GetWidth () == 0 && currentPresentable->GetWindow ().GetHeight () == 0) {
        return;
    }

    try {
        std::shared_ptr<const Stimulus> stim = sequence->getStimulusAtFrame (frameIndex);
        if (GVK_VERIFY (stim != nullptr)) {
            const size_t nextResourceIndex = renderer->GetNextRenderResourceIndex ();
            resourceIndexToRenderedFrameMapping[nextResourceIndex] = frameIndex;
            views[stim]->RenderFrameIndex (*renderer, currentPresentable, stim, frameIndex, *this, *randomExporter);
        }
    } catch (GVK::OutOfDateSwapchain&) {
        if (currentPresentable->GetWindow ().GetWidth () == 0 && currentPresentable->GetWindow ().GetHeight () == 0) {
            return;
        }
        environment.Wait ();
        views.clear ();
        currentPresentable->GetSwapchain ().Recreate ();
        CreateStimulusAdapterViews ();
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

    resourceIndexToRenderedFrameMapping.clear ();
    resourceIndexToRenderedFrameMapping.resize (renderer->GetFramesInFlight (), 0);
}


std::shared_ptr<GVK::Presentable> SequenceAdapter::GetCurrentPresentable ()
{
    return currentPresentable;
}


void SequenceAdapter::RenderFullOnExternalWindow ()
{
    std::unique_ptr<GVK::Presentable> presentable = std::make_unique<GVK::Presentable> (environment, std::make_unique<GVK::HiddenGLFWWindow> (), std::make_unique<GVK::DefaultSwapchainSettings> ());

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