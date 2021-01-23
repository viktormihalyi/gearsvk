#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

#include <unordered_map>

#include "Assert.hpp"
#include "DeviceExtra.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

class Image;

class CommandBuffer;

USING_PTR (Command);
class GVK_RENDERER_API Command {
public:
    virtual ~Command () = default;

    virtual void        Record (CommandBuffer&) = 0;
    virtual std::string ToString () const { return ""; }
};

USING_PTR (CommandBuffer);
class GVK_RENDERER_API CommandBuffer : public VulkanObject {
private:
    const VkDevice      device;
    const VkCommandPool commandPool;
    VkCommandBuffer     handle;

    std::unordered_map<const Image*, std::vector<std::pair<VkImageLayout, VkImageLayout>>> layouts;

public:
    bool                  canRecordCommands;
    std::vector<CommandU> recordedAbstractCommands;

public:
    USING_CREATE (CommandBuffer);

    CommandBuffer (VkDevice device, VkCommandPool commandPool)
        : device (device)
        , commandPool (commandPool)
        , handle (VK_NULL_HANDLE)
        , canRecordCommands (false)
    {
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
        commandBufferAllocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool                 = commandPool;
        commandBufferAllocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount          = 1;

        if (GVK_ERROR (vkAllocateCommandBuffers (device, &commandBufferAllocInfo, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to allocate command buffer");
        }
    }

    CommandBuffer (const DeviceExtra& device)
        : CommandBuffer (device, device.GetCommandPool ())
    {
    }

    virtual ~CommandBuffer () override
    {
        vkFreeCommandBuffers (device, commandPool, 1, &handle);
        handle = VK_NULL_HANDLE;
    }

    void Begin (VkCommandBufferUsageFlags flags = 0)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = flags;
        beginInfo.pInheritanceInfo         = nullptr;

        if (GVK_ERROR (vkBeginCommandBuffer (handle, &beginInfo) != VK_SUCCESS)) {
            throw std::runtime_error ("commandbuffer begin failed");
        }

        canRecordCommands = true;
    }

    void End ()
    {
        if (GVK_ERROR (vkEndCommandBuffer (handle) != VK_SUCCESS)) {
            throw std::runtime_error ("commandbuffer end failed");
        }

        canRecordCommands = false;
    }

    void Reset (bool releaseResources = true)
    {
        if (GVK_ERROR (vkResetCommandBuffer (handle, (releaseResources) ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0) != VK_SUCCESS)) {
            throw std::runtime_error ("commandbuffer reset failed");
        }

        canRecordCommands = false;
    }

    void Record (CommandU&& command)
    {
        command->Record (*this);
        recordedAbstractCommands.push_back (std::move (command));
    }

    template<typename CommandType, typename... CommandParameters>
    void RecordT (CommandParameters&&... parameters)
    {
        Record (std::make_unique<CommandType> (std::forward<CommandParameters> (parameters)...));
    }

    VkCommandBuffer GetHandle () const
    {
        return handle;
    }
};


USING_PTR (CommandBindVertexBuffers);
class GVK_RENDERER_API CommandBindVertexBuffers : public Command {
    USING_CREATE (CommandBindVertexBuffers);

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


USING_PTR (CommandPipelineBarrier);
class GVK_RENDERER_API CommandPipelineBarrier : public Command {
    USING_CREATE (CommandPipelineBarrier);

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


USING_PTR (CommandGeneric);
class GVK_RENDERER_API CommandGeneric : public Command {
    USING_CREATE (CommandGeneric);

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


#endif