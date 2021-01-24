#ifndef STIMULUSADAPTERVIEW_HPP
#define STIMULUSADAPTERVIEW_HPP

// from GearsVk
#include "Noncopyable.hpp"
#include "Ptr.hpp"

// from std
#include <map>


class VulkanEnvironment;
class Presentable;
class StimulusAdapterForPresentable;
class Stimulus;


USING_PTR (StimulusAdapterView);
class StimulusAdapterView : public Noncopyable {
    USING_CREATE (StimulusAdapterView);

private:
    VulkanEnvironment&   environment;
    const PtrC<Stimulus> stimulus;

    std::map<Ptr<Presentable>, Ptr<StimulusAdapterForPresentable>> compiledAdapters;

public:
    StimulusAdapterView (VulkanEnvironment& environment, const PtrC<Stimulus>& stimulus);

    void CreateForPresentable (Ptr<Presentable>& presentable);

    void RenderFrameIndex (Ptr<Presentable>& presentable, const uint32_t frameIndex);

    uint32_t GetStartingFrame () const;

    uint32_t GetEndingFrame () const;
};

#endif