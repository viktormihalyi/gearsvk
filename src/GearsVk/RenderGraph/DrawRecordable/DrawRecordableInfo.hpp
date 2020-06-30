#ifndef DRAWRECORDABLEINFO_HPP
#define DRAWRECORDABLEINFO_HPP

#include "DrawRecordable.hpp"

#include <vulkan/vulkan.h>

#include "BufferTransferable.hpp"


USING_PTR (DrawRecordableInfo);

struct DrawRecordableInfo : public DrawRecordable {
public:
    const uint32_t instanceCount;

    const uint32_t                                       vertexCount;
    const std::vector<VkBuffer>                          vertexBuffer;
    const std::vector<VkVertexInputBindingDescription>   vertexInputBindings;
    const std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

    const uint32_t indexCount;
    const VkBuffer indexBuffer;

    USING_CREATE (DrawRecordableInfo);

    DrawRecordableInfo (const uint32_t                                        instanceCount,
                        uint32_t                                              vertexCount,
                        VkBuffer                                              vertexBuffer          = VK_NULL_HANDLE,
                        const std::vector<VkVertexInputBindingDescription>&   vertexInputBindings   = {},
                        const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributes = {},
                        uint32_t                                              indexCount            = 0,
                        VkBuffer                                              indexBuffer           = VK_NULL_HANDLE)
        : instanceCount (instanceCount)
        , vertexCount (vertexCount)
        , vertexBuffer ((vertexBuffer == VK_NULL_HANDLE) ? std::vector<VkBuffer> {} : std::vector<VkBuffer> {vertexBuffer})
        , vertexInputBindings (vertexInputBindings)
        , vertexInputAttributes (vertexInputAttributes)
        , indexCount (indexCount)
        , indexBuffer (indexBuffer)
    {
    }

    DrawRecordableInfo (const uint32_t                         instanceCount,
                        const VertexBufferTransferableUntyped& vertexBuffer,
                        const IndexBufferTransferable&         indexBuffer)
        : instanceCount (instanceCount)
        , vertexCount (vertexBuffer.data.size ())
        , vertexBuffer ({vertexBuffer.buffer.GetBufferToBind ()})
        , vertexInputBindings (vertexBuffer.info.bindings)
        , vertexInputAttributes (vertexBuffer.info.attributes)
        , indexCount (indexBuffer.data.size ())
        , indexBuffer (indexBuffer.buffer.GetBufferToBind ())
    {
    }

    DrawRecordableInfo (const uint32_t                         instanceCount,
                        const VertexBufferTransferableUntyped& vertexBuffer)
        : instanceCount (instanceCount)
        , vertexCount (vertexBuffer.data.size ())
        , vertexBuffer ({vertexBuffer.buffer.GetBufferToBind ()})
        , vertexInputBindings (vertexBuffer.info.bindings)
        , vertexInputAttributes (vertexBuffer.info.attributes)
        , indexCount (0)
        , indexBuffer (VK_NULL_HANDLE)
    {
    }

    void Record (VkCommandBuffer commandBuffer) const override
    {
        ASSERT (instanceCount == 1);

        if (!vertexBuffer.empty ()) {
            std::vector<VkDeviceSize> offsets (vertexBuffer.size (), 0);
            vkCmdBindVertexBuffers (commandBuffer, 0, vertexBuffer.size (), vertexBuffer.data (), offsets.data ());
        }

        if (indexBuffer != VK_NULL_HANDLE) {
            vkCmdBindIndexBuffer (commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        }

        if (indexBuffer != VK_NULL_HANDLE) {
            vkCmdDrawIndexed (commandBuffer, indexCount, instanceCount, 0, 0, 0);
        } else {
            vkCmdDraw (commandBuffer, vertexCount, instanceCount, 0, 0);
        }
    }

    virtual std::vector<VkVertexInputAttributeDescription> GetAttributes () const
    {
        return vertexInputAttributes;
    }

    virtual std::vector<VkVertexInputBindingDescription> GetBindings () const
    {
        return vertexInputBindings;
    }
};

class DrawRecordableInfoProvider : public DrawRecordable {
public:
    void                                           Record (VkCommandBuffer commandBuffer) const override { GetDrawRecordableInfo ().Record (commandBuffer); }
    std::vector<VkVertexInputAttributeDescription> GetAttributes () const override { return GetDrawRecordableInfo ().GetAttributes (); }
    std::vector<VkVertexInputBindingDescription>   GetBindings () const override { return GetDrawRecordableInfo ().GetBindings (); }

private:
    virtual const DrawRecordableInfo& GetDrawRecordableInfo () const = 0;
};


#endif