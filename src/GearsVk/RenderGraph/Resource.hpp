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
    enum class ReadUsage {
        ShaderReadOnly,
        Present
    };

    enum class WriteUsage {
        ColorAttachment,
        TransferDestination,
    };

    USING_PTR_ABSTRACT (Resource);

    virtual ~Resource () {}

    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const = 0;

    virtual void BindRead (VkCommandBuffer commandBuffer)  = 0;
    virtual void BindWrite (VkCommandBuffer commandBuffer) = 0;
};

struct ImageResource final : public Resource {
    AllocatedImage image;
    ImageView::U   imageView;
    Sampler::U     sampler;


    // write always happens before read
    // NO  read, NO  write: general
    // YES read, NO  write: general -> read
    // NO  read, YES write: general -> write
    // YES read, YES write: general -> write -> read

    std::optional<VkImageLayout> layoutRead;
    std::optional<VkImageLayout> layoutWrite;

    USING_PTR (ImageResource);

    ImageResource (const GraphInfo& graphInfo, const Device& device, VkQueue queue, VkCommandPool commandPool, std::optional<VkImageLayout> layoutRead, std::optional<VkImageLayout> layoutWrite);

    virtual ~ImageResource () {}

    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const override;

    virtual void BindRead (VkCommandBuffer commandBuffer);
    virtual void BindWrite (VkCommandBuffer commandBuffer);
};


struct ResourceVisitor final {
public:
    static void Visit (Resource& res, const std::function<void (ImageResource&)>& imageResourceTypeCallback);
};

} // namespace RenderGraph

#endif