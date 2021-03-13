#ifndef DRAWRECORDABLEINFO_HPP
#define DRAWRECORDABLEINFO_HPP

#include "DrawRecordable.hpp"
#include <memory>

#include <vulkan/vulkan.h>

#include "BufferTransferable.hpp"

namespace GVK {

class VertexBufferList {
private:
    template<typename T>
    std::vector<T> Get (std::function<T (const std::shared_ptr<VertexBufferTransferableUntyped>&)> getterFunc) const
    {
        std::vector<T> result;

        for (auto& vb : vertexBuffers) {
            result.push_back (getterFunc (vb));
        }

        return result;
    }

    template<typename T>
    std::vector<T> GetFromVector (std::function<std::vector<T> (const std::shared_ptr<VertexBufferTransferableUntyped>&)> getterFunc) const
    {
        std::vector<T> result;

        for (auto& vb : vertexBuffers) {
            std::vector<T> res = getterFunc (vb);
            result.insert (result.end (), res.begin (), res.end ());
        }

        return result;
    }

public:
    std::vector<std::shared_ptr<VertexBufferTransferableUntyped>> vertexBuffers;

    VertexBufferList () = default;

    VertexBufferList (std::vector<std::shared_ptr<VertexBufferTransferableUntyped>> vertexBuffers)
        : vertexBuffers (vertexBuffers)
    {
    }

    void Add (std::shared_ptr<VertexBufferTransferableUntyped> vb)
    {
        vertexBuffers.push_back (vb);
    }

    std::vector<VkBuffer> GetHandles () const
    {
        return Get<VkBuffer> ([] (const std::shared_ptr<VertexBufferTransferableUntyped>& vb) {
            return vb->buffer.GetBufferToBind ();
        });
    }

    std::vector<VkVertexInputBindingDescription> GetBindings () const
    {
        uint32_t nextBinding = 0;
        return GetFromVector<VkVertexInputBindingDescription> ([&] (const std::shared_ptr<VertexBufferTransferableUntyped>& vb) {
            return vb->info.GetBindings (nextBinding++);
        });
    }

    std::vector<VkVertexInputAttributeDescription> GetAttributes () const
    {
        size_t nextLocation = 0;
        size_t nextBinding  = 0;
        return GetFromVector<VkVertexInputAttributeDescription> ([&] (const std::shared_ptr<VertexBufferTransferableUntyped>& vb) {
            auto attribs = vb->info.GetAttributes (nextLocation, nextBinding++);
            nextLocation += attribs.size ();
            return attribs;
        });
    }
};


class DrawRecordableInfo : public DrawRecordable {
public:
    const uint32_t instanceCount;

    const uint32_t                                       vertexCount;
    const std::vector<VkBuffer>                          vertexBuffer;
    const std::vector<VkVertexInputBindingDescription>   vertexInputBindings;
    const std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

    const uint32_t indexCount;
    const VkBuffer indexBuffer;

    DrawRecordableInfo (const uint32_t                                        instanceCount,
                        uint32_t                                              vertexCount,
                        VkBuffer                                              vertexBuffer          = VK_NULL_HANDLE,
                        const std::vector<VkVertexInputBindingDescription>&   vertexInputBindings   = {},
                        const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributes = {},
                        uint32_t                                              indexCount            = 0,
                        VkBuffer                                              indexBuffer           = VK_NULL_HANDLE)
        : instanceCount (instanceCount)
        , vertexCount (vertexCount)
        , vertexBuffer ((vertexBuffer == VK_NULL_HANDLE) ? std::vector<VkBuffer> {} : std::vector<VkBuffer> { vertexBuffer })
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
        , vertexBuffer ({ vertexBuffer.buffer.GetBufferToBind () })
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
        , vertexBuffer ({ vertexBuffer.buffer.GetBufferToBind () })
        , vertexInputBindings (vertexBuffer.info.bindings)
        , vertexInputAttributes (vertexBuffer.info.attributes)
        , indexCount (0)
        , indexBuffer (VK_NULL_HANDLE)
    {
    }

    DrawRecordableInfo (const uint32_t   instanceCount,
                        uint32_t         vertexCount,
                        VertexBufferList vertexBuffers,
                        uint32_t         indexCount,
                        VkBuffer         indexBuffer)
        : instanceCount (instanceCount)
        , vertexCount (vertexCount)
        , vertexBuffer (vertexBuffers.GetHandles ())
        , vertexInputBindings (vertexBuffers.GetBindings ())
        , vertexInputAttributes (vertexBuffers.GetAttributes ())
        , indexCount (indexCount)
        , indexBuffer (indexBuffer)
    {
    }

    void Record (CommandBuffer& commandBuffer) const override
    {
        if (!vertexBuffer.empty ()) {
            std::vector<VkDeviceSize> offsets (vertexBuffer.size (), 0);
            commandBuffer.RecordT<CommandBindVertexBuffers> (0, vertexBuffer.size (), vertexBuffer, offsets);
        }

        if (indexBuffer != VK_NULL_HANDLE) {
            commandBuffer.RecordT<CommandGeneric> ([=] (VkCommandBuffer commandBuffer) { vkCmdBindIndexBuffer (commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16); });
        }

        if (indexBuffer != VK_NULL_HANDLE) {
            commandBuffer.RecordT<CommandGeneric> ([=] (VkCommandBuffer commandBuffer) { vkCmdDrawIndexed (commandBuffer, indexCount, instanceCount, 0, 0, 0); });
        } else {
            commandBuffer.RecordT<CommandGeneric> ([=] (VkCommandBuffer commandBuffer) { vkCmdDraw (commandBuffer, vertexCount, instanceCount, 0, 0); });
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
    void                                           Record (CommandBuffer& commandBuffer) const override { GetDrawRecordableInfo ().Record (commandBuffer); }
    std::vector<VkVertexInputAttributeDescription> GetAttributes () const override { return GetDrawRecordableInfo ().GetAttributes (); }
    std::vector<VkVertexInputBindingDescription>   GetBindings () const override { return GetDrawRecordableInfo ().GetBindings (); }

private:
    virtual const DrawRecordableInfo& GetDrawRecordableInfo () const = 0;
};

} // namespace GVK

#endif