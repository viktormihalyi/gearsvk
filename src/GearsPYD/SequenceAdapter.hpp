#ifndef SEQUENCEADAPTER_HPP
#define SEQUENCEADAPTER_HPP


// from GearsVk
#include "Ptr.hpp"

// from Gears
#include "GearsAPI.hpp"
#include "core/Sequence.h"
#include "core/Stimulus.h"

// from std
#include <optional>


class VulkanEnvironment;
class Presentable;
class StimulusAdapterView;


USING_PTR (SequenceAdapter);
class GEARS_TEST_API SequenceAdapter {
    USING_CREATE (SequenceAdapter);

private:
    const Sequence::P sequence;

    std::optional<uint32_t> lastRenderedFrameIndex;

    VulkanEnvironment& environment;
    Ptr<Presentable>   currentPresentable;

    std::map<Stimulus::CP, Ptr<StimulusAdapterView>> views;

public:
    SequenceAdapter (VulkanEnvironment& environment, const Sequence::P& sequence);

    void RenderFrameIndex (const uint32_t frameIndex);

    void Wait ();

    void SetCurrentPresentable (Ptr<Presentable> presentable);

    Ptr<Presentable> GetCurrentPresentable ();

    void RenderFullOnExternalWindow ();
};


#endif