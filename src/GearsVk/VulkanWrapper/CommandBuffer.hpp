#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "DeviceExtra.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

USING_PTR (CommandBuffer);

class GEARSVK_API CommandBuffer : public VulkanObject {
private:
    const VkDevice      device;
    const VkCommandPool commandPool;
    VkCommandBuffer     handle;

public:
    enum class CommandType : uint8_t {
        BindVertexBuffers = 0,
        BindIndexBuffer,
        DrawIndexed,
        Draw,
        BeginRenderPass,
        BindPipeline,
        BindDescriptorSets,
        EndRenderPass,
        PipelineBarrier,
        CopyBufferToImage,
        CopyImage,
        CopyBuffer,
    };

    static std::string CommandTypeToString (CommandType commandType)
    {
        static const std::vector<std::string> commandTypeStrings = {
            "BindVertexBuffers",
            "BindIndexBuffer",
            "DrawIndexed",
            "Draw",
            "BeginRenderPass",
            "BindPipeline",
            "BindDescriptorSets",
            "EndRenderPass",
            "PipelineBarrier",
            "CopyBufferToImage",
            "CopyImage",
            "CopyBuffer",
        };

        return commandTypeStrings.at (static_cast<uint32_t> (commandType));
    }

    bool                     canRecordCommands;
    std::vector<CommandType> recordedCommands;

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

    ~CommandBuffer ()
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

    void CmdPipelineBarrier (VkPipelineStageFlags               srcStageMask,
                             VkPipelineStageFlags               dstStageMask,
                             std::vector<VkMemoryBarrier>       memoryBarriers       = {},
                             std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers = {},
                             std::vector<VkImageMemoryBarrier>  imageMemoryBarriers  = {})
    {
        vkCmdPipelineBarrier (
            handle,
            srcStageMask,
            dstStageMask,
            0, // TODO
            static_cast<uint32_t> (memoryBarriers.size ()), memoryBarriers.data (),
            static_cast<uint32_t> (bufferMemoryBarriers.size ()), bufferMemoryBarriers.data (),
            static_cast<uint32_t> (imageMemoryBarriers.size ()), imageMemoryBarriers.data ());

        recordedCommands.push_back (CommandType::PipelineBarrier);
    }

#define GVK_CMDBUFFER_DEFINE_CMD(commandName)                                  \
    template<typename... Parameters>                                           \
    void Cmd##commandName (Parameters&&... parameters)                         \
    {                                                                          \
        GVK_ASSERT (canRecordCommands);                                        \
        vkCmd##commandName (handle, std::forward<Parameters> (parameters)...); \
        recordedCommands.push_back (CommandType::commandName);                 \
    }

    GVK_CMDBUFFER_DEFINE_CMD (BindVertexBuffers);
    GVK_CMDBUFFER_DEFINE_CMD (BindIndexBuffer);
    GVK_CMDBUFFER_DEFINE_CMD (DrawIndexed);
    GVK_CMDBUFFER_DEFINE_CMD (Draw);
    GVK_CMDBUFFER_DEFINE_CMD (BeginRenderPass);
    GVK_CMDBUFFER_DEFINE_CMD (BindPipeline);
    GVK_CMDBUFFER_DEFINE_CMD (BindDescriptorSets);
    GVK_CMDBUFFER_DEFINE_CMD (EndRenderPass);
    GVK_CMDBUFFER_DEFINE_CMD (CopyBufferToImage);
    GVK_CMDBUFFER_DEFINE_CMD (CopyImage);
    GVK_CMDBUFFER_DEFINE_CMD (CopyBuffer);

    VkCommandBuffer GetHandle () const
    {
        return handle;
    }
};

#endif