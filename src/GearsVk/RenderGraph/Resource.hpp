#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "GraphInfo.hpp"

namespace RenderGraph {

class Resource : public Noncopyable {
public:
    USING_PTR_ABSTRACT (Resource);

    virtual ~Resource () {}

    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const = 0;
};

struct ImageResource final : public Resource {
    AllocatedImage image;
    ImageView::U   imageView;
    Sampler::U     sampler;

    USING_PTR (ImageResource);

    ImageResource (const GraphInfo& graphInfo, const Device& device, VkQueue queue, VkCommandPool commandPool);

    virtual ~ImageResource () {}

    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const override;
};


struct ResourceVisitor final {
public:
    static void Visit (Resource& res, const std::function<void (ImageResource&)>& imageResourceTypeCallback);
};

} // namespace RenderGraph

#endif