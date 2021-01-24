#ifndef SEQUENCEADAPTER_HPP
#define SEQUENCEADAPTER_HPP

// from GearsVk
#include "Ptr.hpp"

// from Gears
#include "GearsAPI.hpp"

// from std
#include <map>
#include <optional>


class VulkanEnvironment;
class Presentable;
class StimulusAdapterView;
class Stimulus;
class Sequence;


USING_PTR (SequenceAdapter);
class GEARS_API_TEST SequenceAdapter {
    USING_CREATE (SequenceAdapter);

private:
    const Ptr<Sequence> sequence;

    std::optional<uint32_t> lastRenderedFrameIndex;

    VulkanEnvironment& environment;
    Ptr<Presentable>   currentPresentable;

    std::map<PtrC<Stimulus>, Ptr<StimulusAdapterView>> views;

public:
    SequenceAdapter (VulkanEnvironment& environment, const Ptr<Sequence>& sequence);

    void RenderFrameIndex (const uint32_t frameIndex);

    void Wait ();

    void SetCurrentPresentable (Ptr<Presentable> presentable);

    Ptr<Presentable> GetCurrentPresentable ();

    void RenderFullOnExternalWindow ();
};


#endif