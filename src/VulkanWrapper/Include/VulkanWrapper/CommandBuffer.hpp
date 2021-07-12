#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

#include <unordered_map>

#include "DeviceExtra.hpp"
#include "Utils/MovablePtr.hpp"
#include "Utils/Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class Image;

class CommandBuffer;

class VULKANWRAPPER_API Command {
private:
    std::string name;

public:
    virtual ~Command () = default;

    void SetName (const std::string& value) { name = value; }

    virtual void        Record (CommandBuffer&) = 0;
    virtual std::string ToString () const { return ""; }
};

class VULKANWRAPPER_API CommandBuffer : public VulkanObject {
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

    Command& Record (std::unique_ptr<Command>&& command);

    template<typename CommandType, typename... CommandParameters>
    Command& RecordT (CommandParameters&&... parameters)
    {
        return Record (std::make_unique<CommandType> (std::forward<CommandParameters> (parameters)...));
    }

    VkCommandBuffer GetHandle () const { return handle; }
};


class VULKANWRAPPER_API CommandBindVertexBuffers : public Command {
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


class VULKANWRAPPER_API CommandPipelineBarrier : public Command {
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


class VULKANWRAPPER_API CommandGeneric : public Command {
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


class VULKANWRAPPER_API CommandDrawIndexed : public Command {
private:
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t  vertexOffset;
    uint32_t firstInstance;

public:
    CommandDrawIndexed (uint32_t indexCount,
                        uint32_t instanceCount,
                        uint32_t firstIndex,
                        int32_t  vertexOffset,
                        uint32_t firstInstance)
        : indexCount (indexCount)
        , instanceCount (instanceCount)
        , firstIndex (firstIndex)
        , vertexOffset (vertexOffset)
        , firstInstance (firstInstance)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdDrawIndexed (commandBuffer.GetHandle (), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
};


class VULKANWRAPPER_API CommandDraw : public Command {
private:
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;

public:
    CommandDraw (uint32_t vertexCount,
                 uint32_t instanceCount,
                 uint32_t firstVertex,
                 uint32_t firstInstance)
        : vertexCount (vertexCount)
        , instanceCount (instanceCount)
        , firstVertex (firstVertex)
        , firstInstance (firstInstance)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdDraw (commandBuffer.GetHandle (), vertexCount, instanceCount, firstVertex, firstInstance);
    }
};


class VULKANWRAPPER_API CommandBindIndexBuffer : public Command {
private:
    VkBuffer     buffer;
    VkDeviceSize offset;
    VkIndexType  indexType;

public:
    CommandBindIndexBuffer (VkBuffer     buffer,
                            VkDeviceSize offset,
                            VkIndexType  indexType)
        : buffer (buffer)
        , offset (offset)
        , indexType (indexType)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdBindIndexBuffer (commandBuffer.GetHandle (), buffer, offset, indexType);
    }
};


class VULKANWRAPPER_API CommandEndRenderPass : public Command {
public:
    CommandEndRenderPass () = default;

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdEndRenderPass (commandBuffer.GetHandle ());
    }
};


class VULKANWRAPPER_API CommandBeginRenderPass : public Command {
private:
    VkRenderPassBeginInfo renderPassBegin;
    VkSubpassContents     contents;

public:
    CommandBeginRenderPass (VkRenderPassBeginInfo renderPassBegin,
                            VkSubpassContents     contents)
        : renderPassBegin (renderPassBegin)
        , contents (contents)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdBeginRenderPass (commandBuffer.GetHandle (), &renderPassBegin, contents);
    }
};


class VULKANWRAPPER_API CommandBindPipeline : public Command {
private:
    VkPipelineBindPoint pipelineBindPoint;
    VkPipeline          pipeline;

public:
    CommandBindPipeline (VkPipelineBindPoint pipelineBindPoint,
                         VkPipeline          pipeline)
        : pipelineBindPoint (pipelineBindPoint)
        , pipeline (pipeline)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdBindPipeline (commandBuffer.GetHandle (), pipelineBindPoint, pipeline);
    }
};


class VULKANWRAPPER_API CommandBindDescriptorSets : public Command {
private:
    VkPipelineBindPoint          pipelineBindPoint;
    VkPipelineLayout             layout;
    uint32_t                     firstSet;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<uint32_t>        dynamicOffsets;

public:
    CommandBindDescriptorSets (VkPipelineBindPoint                 pipelineBindPoint,
                               VkPipelineLayout                    layout,
                               uint32_t                            firstSet,
                               const std::vector<VkDescriptorSet>& descriptorSets,
                               const std::vector<uint32_t>&        dynamicOffsets)
        : pipelineBindPoint (pipelineBindPoint)
        , layout (layout)
        , firstSet (firstSet)
        , descriptorSets (descriptorSets)
        , dynamicOffsets (dynamicOffsets)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdBindDescriptorSets (commandBuffer.GetHandle (), pipelineBindPoint, layout, firstSet,
                                 descriptorSets.size (), descriptorSets.data (),
                                 dynamicOffsets.size (), dynamicOffsets.data ());
    }
};

class VULKANWRAPPER_API CommandCopyImage : public Command {
private:
    VkImage                  srcImage;
    VkImageLayout            srcImageLayout;
    VkImage                  dstImage;
    VkImageLayout            dstImageLayout;
    std::vector<VkImageCopy> regions;

public:
    CommandCopyImage (VkImage                         srcImage,
                      VkImageLayout                   srcImageLayout,
                      VkImage                         dstImage,
                      VkImageLayout                   dstImageLayout,
                      const std::vector<VkImageCopy>& regions)
        : srcImage (srcImage)
        , srcImageLayout (srcImageLayout)
        , dstImage (dstImage)
        , dstImageLayout (dstImageLayout)
        , regions (regions)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdCopyImage (commandBuffer.GetHandle (), srcImage, srcImageLayout, dstImage, dstImageLayout, regions.size (), regions.data ());
    }
};

class VULKANWRAPPER_API CommandCopyImageToBuffer : public Command {
private:
    VkImage                        srcImage;
    VkImageLayout                  srcImageLayout;
    VkBuffer                       dstBuffer;
    std::vector<VkBufferImageCopy> regions;

public:
    CommandCopyImageToBuffer (VkImage                               srcImage,
                              VkImageLayout                         srcImageLayout,
                              VkBuffer                              dstBuffer,
                              const std::vector<VkBufferImageCopy>& regions)
        : srcImage (srcImage)
        , srcImageLayout (srcImageLayout)
        , dstBuffer (dstBuffer)
        , regions (regions)
    {
    }


    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdCopyImageToBuffer (commandBuffer.GetHandle (),
                                srcImage,
                                srcImageLayout,
                                dstBuffer,
                                regions.size (), regions.data ());
    }
};


class VULKANWRAPPER_API CommandCopyBufferToImage : public Command {
private:
    VkBuffer                       srcBuffer;
    VkImage                        dstImage;
    VkImageLayout                  dstImageLayout;
    std::vector<VkBufferImageCopy> regions;

public:
    CommandCopyBufferToImage (VkBuffer                       srcBuffer,
                              VkImage                        dstImage,
                              VkImageLayout                  dstImageLayout,
                              std::vector<VkBufferImageCopy> regions)
        : srcBuffer (srcBuffer)
        , dstImage (dstImage)
        , dstImageLayout (dstImageLayout)
        , regions (regions)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdCopyBufferToImage (commandBuffer.GetHandle (), srcBuffer, dstImage, dstImageLayout, regions.size (), regions.data ());
    }
};


class VULKANWRAPPER_API CommandCopyBuffer : public Command {
private:
    VkBuffer                  srcBuffer;
    VkBuffer                  dstBuffer;
    std::vector<VkBufferCopy> regions;

public:
    CommandCopyBuffer (VkBuffer                         srcBuffer,
                       VkBuffer                         dstBuffer,
                       const std::vector<VkBufferCopy>& regions)
        : srcBuffer (srcBuffer)
        , dstBuffer (dstBuffer)
        , regions (regions)
    {
    }

    virtual void Record (CommandBuffer& commandBuffer) override
    {
        vkCmdCopyBuffer (commandBuffer.GetHandle (), srcBuffer, dstBuffer, regions.size (), regions.data ());
    }
};

} // namespace GVK

#endif