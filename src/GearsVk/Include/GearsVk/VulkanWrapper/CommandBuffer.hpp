#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

#include <unordered_map>

#include "DeviceExtra.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class Image;

class CommandBuffer;

class GVK_RENDERER_API Command {
public:
    virtual ~Command () = default;

    virtual void        Record (CommandBuffer&) = 0;
    virtual std::string ToString () const { return ""; }
};

class GVK_RENDERER_API CommandBuffer : public VulkanObject {
private:
    GVK::MovablePtr<VkDevice>        device;
    GVK::MovablePtr<VkCommandPool>   commandPool;
    GVK::MovablePtr<VkCommandBuffer> handle;

    bool                                  canRecordCommands;
    std::vector<std::unique_ptr<Command>> recordedAbstractCommands;

public:
    CommandBuffer (VkDevice device, VkCommandPool commandPool);
    CommandBuffer (const DeviceExtra& device);

    CommandBuffer (CommandBuffer&&) = default;
    CommandBuffer& operator= (CommandBuffer&&) = default;

    virtual ~CommandBuffer () override;

    void Begin (VkCommandBufferUsageFlags flags = 0);

    void End ();

    void Reset (bool releaseResources = true);

    void Record (std::unique_ptr<Command>&& command);

    template<typename CommandType, typename... CommandParameters>
    void RecordT (CommandParameters&&... parameters)
    {
        Record (std::make_unique<CommandType> (std::forward<CommandParameters> (parameters)...));
    }

    VkCommandBuffer GetHandle () const { return handle; }
};


class GVK_RENDERER_API CommandBindVertexBuffers : public Command {
private:
    const uint32_t                  firstBinding;
    const uint32_t                  bindingCount;
    const std::vector<VkBuffer>     pBuffers;
    const std::vector<VkDeviceSize> pOffsets;

public:
    CommandBindVertexBuffers (const uint32_t                  firstBinding,
                              const uint32_t                  bindingCount,
                              const std::vector<VkBuffer>     pBuffers,
                              const std::vector<VkDeviceSize> pOffsets)
        : firstBinding (firstBinding)
        , bindingCount (bindingCount)
        , pBuffers (pBuffers)
        , pOffsets (pOffsets)
    {
    }

    virtual void        Record (CommandBuffer&) override;
    virtual std::string ToString () const override;
};


class GVK_RENDERER_API CommandPipelineBarrier : public Command {
private:
    const VkPipelineStageFlags               srcStageMask;
    const VkPipelineStageFlags               dstStageMask;
    const std::vector<VkMemoryBarrier>       memoryBarriers;
    const std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
    const std::vector<VkImageMemoryBarrier>  imageMemoryBarriers;

public:
    CommandPipelineBarrier (const VkPipelineStageFlags               srcStageMask,
                            const VkPipelineStageFlags               dstStageMask,
                            const std::vector<VkMemoryBarrier>       memoryBarriers       = {},
                            const std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers = {},
                            const std::vector<VkImageMemoryBarrier>  imageMemoryBarriers  = {})
        : srcStageMask (srcStageMask)
        , dstStageMask (dstStageMask)
        , memoryBarriers (memoryBarriers)
        , bufferMemoryBarriers (bufferMemoryBarriers)
        , imageMemoryBarriers (imageMemoryBarriers)
    {
    }

    virtual void Record (CommandBuffer&) override;
};


class GVK_RENDERER_API CommandGeneric : public Command {
private:
    std::function<void (VkCommandBuffer)> recordCallback;

public:
    CommandGeneric (const std::function<void (VkCommandBuffer)>& recordCallback)
        : recordCallback (recordCallback)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        recordCallback (commandBuffer.GetHandle ());
    }
};

} // namespace GVK

#endif