#ifndef SEQUENCEADAPTER_HPP
#define SEQUENCEADAPTER_HPP


// from GearsVk
#include "Ptr.hpp"

// from Gears
#include "core/Sequence.h"
#include "core/Stimulus.h"


class VulkanEnvironment;
class Presentable;
class StimulusAdapterView;


USING_PTR (SequenceAdapter);
class SequenceAdapter {
    USING_CREATE (SequenceAdapter);

private:
    const Sequence::P sequence;


    VulkanEnvironment& environment;
    Ptr<Presentable>   currentPresentable;

    std::map<Stimulus::CP, Ptr<StimulusAdapterView>> views;

public:
    SequenceAdapter (VulkanEnvironment& environment, const Sequence::P& sequence);

    void RenderFrameIndex (const uint32_t frameIndex);

    void SetCurrentPresentable (Ptr<Presentable> presentable);

    Ptr<Presentable> GetCurrentPresentable ();

    void RenderFullOnExternalWindow ();
};


#endif