#include "SequenceAdapter.hpp"

// from RenderGraph
#include "Utils/Assert.hpp"
#include "Utils/CommandLineFlag.hpp"
#include "Utils/FileSystemUtils.hpp"

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
#include <sstream>
#include <iostream>

#include "spdlog/spdlog.h"


class RandomExporter : public IRandomExporter {
private:
    GVK::DeviceExtra& device;
    std::shared_ptr<Sequence> sequence;

    size_t randomValueLimit;
    size_t histogramBins;

    std::vector<uint32_t> values;
    std::vector<uint32_t> histogram;
    bool                  exported;

public:
    RandomExporter (GVK::DeviceExtra& device, const std::shared_ptr<Sequence>& sequence, size_t randomValueLimit, size_t histogramBins)
        : device { device }
        , sequence { sequence }
        , randomValueLimit { randomValueLimit }
        , histogramBins { histogramBins }
        , exported { false }
    {
        spdlog::info ("RandomExported initialized with limit: {}, bins: {}", randomValueLimit, histogramBins);

        values.reserve (randomValueLimit);
        histogram.resize (histogramBins, 0);
    }

    virtual ~RandomExporter () override
    {
        if (!exported)
            Export ();
    }

    virtual bool IsEnabled () override
    {
        return values.size () < randomValueLimit;
    }

    virtual void OnRandomTextureDrawn (RG::GPUBufferResource& randomsBuffer, uint32_t resourceIndex, uint32_t frameIndex) override
    {
        auto stimulus = sequence->getStimulusAtFrame (frameIndex);

        if (stimulus->rngCompute_multiLayer) {
            // TODO only export one 'layer'
        }
        
        randomsBuffer.TransferFromGPUToCPU (resourceIndex);

        std::vector<uint32_t> as32BitUint;
        as32BitUint.resize (randomsBuffer.GetBufferSize () / sizeof (uint32_t));
        
        const void* const source      = randomsBuffer.buffers[resourceIndex]->bufferCPUMapping.Get ();
        void* const       destination = as32BitUint.data ();

        const size_t bufferOffset = 0;
        const size_t copySize     = randomsBuffer.GetBufferSize ();

        memcpy (destination, source, copySize);

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
        GVK_ASSERT (!exported);

        spdlog::info ("Exporting randoms...");

        const std::filesystem::path dir = std::filesystem::temp_directory_path () / "GearsVk" / "RandomExport";

        {
            const std::filesystem::path valuesFilePath = dir / "values.txt";
            
            spdlog::info ("Exporting values to {} ...", valuesFilePath.string ());
            
            Utils::EnsureParentFolderExists (valuesFilePath);
            std::ofstream valuesFile { valuesFilePath.string (), std::ios::out };
            if (GVK_VERIFY (valuesFile.is_open ())) {
                for (uint32_t rndval : values) {
                    valuesFile << rndval << std::endl;
                }
            }
        }

        {
            const std::filesystem::path valuesFilePath = dir / "values.bin";

            spdlog::info ("Exporting values to {} ...", valuesFilePath.string ());

            Utils::EnsureParentFolderExists (valuesFilePath);
            std::ofstream valuesFile { valuesFilePath.string (), std::ios::binary | std::ios::out };
            if (GVK_VERIFY (valuesFile.is_open ())) {
                for (uint32_t rndval : values) {
                    valuesFile.write (reinterpret_cast<const char*> (&rndval), sizeof (uint32_t));
                }
            }
        }

        {
            const std::filesystem::path histogramFilePath = dir / "histogram.txt";

            std::ofstream histogramFile { histogramFilePath.string (), std::ios::out };
            
            spdlog::info ("Exporting histogram to {} ...", histogramFilePath.string ());
            
            if (GVK_VERIFY (histogramFile.is_open ())) {
                for (size_t i = 0; i < histogram.size (); ++i) {
                    histogramFile << i << " " << histogram[i] << std::endl;
                }
            }
        }

        exported = true;

        spdlog::info ("Exporting randoms... Done!");
    }
};


class NoRandomExporter : public IRandomExporter {
public:
    virtual ~NoRandomExporter () override = default;
    virtual bool IsEnabled () override { return false; }
    virtual void OnRandomTextureDrawn (RG::GPUBufferResource&, uint32_t, uint32_t) override {}
};


Utils::CommandLineOnOffFlag saveRandomsFlag { "--saveRandoms", "Saves random buffers to %temp%/GearsVk/" };

static std::unique_ptr<IRandomExporter> GetRandomExporterImpl (GVK::DeviceExtra& device, const std::shared_ptr<Sequence>& sequence)
{
    if (saveRandomsFlag.IsFlagOn ()) {
        return std::make_unique<RandomExporter> (device, sequence, 50'000'000, 20);
    }

    return std::make_unique<NoRandomExporter> ();
}


Utils::CommandLineOnOffFlag printSignalsFlag { "--printSignals", "Prints Sequence and Stimulus signals to stdout." };


template <typename SignalEventType>
static void PrintSignal (const Sequence&                        sequence,
                         const std::shared_ptr<const Stimulus>& stimulus,
                         const size_t                           frameIndex,
                         const SignalEventType&                 signal)
{
    const std::string channelPort = sequence.getChannels ().find (signal.channel)->second.portName;
    spdlog::info ("Global frame index {} (local: {}) on channel: \"{}\", port: \"{}\", clear: {}",
                  stimulus ? frameIndex + stimulus->getStartingFrame () : frameIndex,
                  stimulus ? std::to_string (frameIndex) : "-",
                  signal.channel,
                  channelPort,
                  signal.clear);
}


static void PrintSignals (const Sequence& sequence)
{
    spdlog::info ("======= Sequence signals BEGIN =======");

    for (const auto& [frameIndex, signal] : sequence.getSignals ())
        PrintSignal (sequence, nullptr, frameIndex, signal);

    spdlog::info ("======= Sequence signals END =========");

    for (const auto& stimulus : sequence.getStimuli ()) {
        spdlog::info ("======= Stimulus signals BEGIN - \"{}\" (starting frame: {}, duration: {}) =======", stimulus.second->name, stimulus.second->getStartingFrame (), stimulus.second->getDuration ());
        for (const auto& [frameIndex, signal] : stimulus.second->getSignals ()) {
            PrintSignal (sequence, stimulus.second, frameIndex, signal);
        }
        spdlog::info ("======= Stimulus signals END =======", stimulus.second->name, stimulus.second->getStartingFrame (), stimulus.second->getDuration ());
    }
}


SequenceAdapter::SequenceAdapter (RG::VulkanEnvironment& environment, const std::shared_ptr<Sequence>& sequence, const std::string& sequenceNameInTitle)
    : sequence { sequence }
    , environment { environment }
    , randomExporter { GetRandomExporterImpl (*environment.deviceExtra, sequence) }
    , sequenceNameInTitle { sequenceNameInTitle }
{
    CreateStimulusAdapterViews ();

    if (printSignalsFlag.IsFlagOn ())
        PrintSignals (*sequence);
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
    
    const std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now ().time_since_epoch ());

    //spdlog::info ("DISPLAYED AT {} ms", ns.count () / 1e6);

    // TODO check if signal's frame index and finishedFrameIndex are the same

    const std::shared_ptr<Stimulus const> stimulus = sequence->getStimulusAtFrame (finishedFrameIndex);
    GVK_ASSERT (stimulus != nullptr);

    GVK_ASSERT (finishedFrameIndex + 1 /* TODO why +1 */ >= stimulus->getStartingFrame ());
    const size_t stimulusFrameIndex = finishedFrameIndex - stimulus->getStartingFrame ();

    if (currentPresentable->HasWindow ()) {
        const std::string titleString = fmt::format ("GearsVk - {} [stimulus frame: {} / {} ({}), sequence frame: {} / {} ({})]",
                                                     sequenceNameInTitle,
                                                     stimulusFrameIndex, stimulus->getDuration (), std::floor (static_cast<double> (stimulusFrameIndex) / stimulus->getDuration () * 100.0),
                                                     finishedFrameIndex, sequence->getDuration (), std::floor (static_cast<double> (finishedFrameIndex) / sequence->getDuration () * 100.0));
        currentPresentable->GetWindow ().SetTitle (titleString);
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

    if (currentPresentable->HasWindow () && currentPresentable->GetWindow ().GetWidth () == 0 && currentPresentable->GetWindow ().GetHeight () == 0) {
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
        if (currentPresentable->HasWindow () && currentPresentable->GetWindow ().GetWidth () == 0 && currentPresentable->GetWindow ().GetHeight () == 0) {
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


void SequenceAdapter::SetCurrentPresentable (std::shared_ptr<RG::Presentable> presentable)
{
    currentPresentable = presentable;

    for (auto& [stim, view] : views) {
        view->CreateForPresentable (currentPresentable);
    }

    renderer = std::make_unique<RG::SynchronizedSwapchainGraphRenderer> (*environment.deviceExtra, presentable->GetSwapchain ());

    resourceIndexToRenderedFrameMapping.clear ();
    resourceIndexToRenderedFrameMapping.resize (renderer->GetFramesInFlight (), 0);
}


void SequenceAdapter::RenderFullOnExternalWindow ()
{
    std::shared_ptr<RG::Presentable> presentable = std::make_unique<RG::Presentable> (environment, std::make_unique<RG::FullscreenGLFWWindow> (), std::make_unique<GVK::DefaultSwapchainSettings> ());

    RG::Window& window = presentable->GetWindow ();

    bool escPressed = false;

    GVK::SingleEventObserver kobs;
    kobs.Observe (window.events.keyPressed, [&] (int32_t key) {
        spdlog::trace ("PRESSED {}", key);

        // F11
        if (key == 300) {
            window.SetWindowMode (window.GetWindowMode () == RG::Window::Mode::Windowed
                                      ? RG::Window::Mode::Fullscreen
                                      : RG::Window::Mode::Windowed);
        }
        
        // esc or q
        if (key == 256 || key == 81) {
            escPressed = true;
        }
    });

    SetCurrentPresentable (std::move (presentable));
    
    window.Show ();

    uint32_t frameIndex = 1; // TODO do all sequences start at frame 1?

    window.DoEventLoop ([&] (bool& shouldStop) {
        spdlog::trace ("DoEventLoop called");

        RenderFrameIndex (frameIndex);
        
        frameIndex++;

        if (frameIndex == sequence->getDuration () || escPressed) {
            shouldStop = true;
            spdlog::trace ("Should stop pressed");
        }
    });

    spdlog::trace ("DoEventLoop ended, env.Wait ()");

    environment.Wait ();

    for (auto& [stim, view] : views) {
        view->DestroyForPresentable (currentPresentable);
    }

    spdlog::trace ("Closing window ");

    currentPresentable = nullptr; 

}