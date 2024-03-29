#ifndef DRAWRECORDABLEINFO_HPP
#define DRAWRECORDABLEINFO_HPP

#include "RenderGraph/RenderGraphAPI.hpp"
#include "RenderGraph/DrawRecordable/DrawRecordable.hpp"

#include "VulkanWrapper/Utils/BufferTransferable.hpp"
#include "VulkanWrapper/CommandBuffer.hpp"

#include <memory>
#include <functional>

#include <vulkan/vulkan.h>

namespace RG {

class VertexBufferList {
private:
    template<typename T>
    std::vector<T> Get (std::function<T (const std::shared_ptr<GVK::VertexBufferTransferableUntyped>&)> getterFunc) const
    {
        std::vector<T> result;

        for (auto& vb : vertexBuffers) {
            result.push_back (getterFunc (vb));
        }

        return result;
    }

    template<typename T>
    std::vector<T> GetFromVector (std::function<std::vector<T> (const std::shared_ptr<GVK::VertexBufferTransferableUntyped>&)> getterFunc) const
    {
        std::vector<T> result;

        for (auto& vb : vertexBuffers) {
            std::vector<T> res = getterFunc (vb);
            result.insert (result.end (), res.begin (), res.end ());
        }

        return result;
    }

public:
    std::vector<std::shared_ptr<GVK::VertexBufferTransferableUntyped>> vertexBuffers;

    VertexBufferList () = default;

    VertexBufferList (std::vector<std::shared_ptr<GVK::VertexBufferTransferableUntyped>> vertexBuffers)
        : vertexBuffers (vertexBuffers)
    {
    }

    void Add (std::shared_ptr<GVK::VertexBufferTransferableUntyped> vb)
    {
        vertexBuffers.push_back (vb);
    }

    std::vector<VkBuffer> GetHandles () const
    {
        return Get<VkBuffer> ([] (const std::shared_ptr<GVK::VertexBufferTransferableUntyped>& vb) {
            return vb->buffer.GetBufferToBind ();
        });
    }
};


class GVK_RENDERER_API DrawRecordableInfo : public DrawRecordable {
public:
    const uint32_t instanceCount;

    const uint32_t                                       vertexCount;
    const std::vector<VkBuffer>                          vertexBuffer;

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
        , indexCount (indexCount)
        , indexBuffer (indexBuffer)
    {
    }

    DrawRecordableInfo (const uint32_t                         instanceCount,
                        const GVK::VertexBufferTransferableUntyped& vertexBuffer,
                        const GVK::IndexBufferTransferable&         indexBuffer)
        : instanceCount (instanceCount)
        , vertexCount (vertexBuffer.data.size ())
        , vertexBuffer ({ vertexBuffer.buffer.GetBufferToBind () })
        , indexCount (indexBuffer.data.size ())
        , indexBuffer (indexBuffer.buffer.GetBufferToBind ())
    {
    }

    DrawRecordableInfo (const uint32_t                         instanceCount,
                        const GVK::VertexBufferTransferableUntyped& vertexBuffer)
        : instanceCount (instanceCount)
        , vertexCount (vertexBuffer.data.size ())
        , vertexBuffer ({ vertexBuffer.buffer.GetBufferToBind () })
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
        , indexCount (indexCount)
        , indexBuffer (indexBuffer)
    {
    }

    virtual ~DrawRecordableInfo () override = default;

    void Record (GVK::CommandBuffer& commandBuffer) const override;
};

class DrawRecordableInfoProvider : public DrawRecordable {
public:
    virtual void Record (GVK::CommandBuffer& commandBuffer) const override { GetDrawRecordableInfo ().Record (commandBuffer); }

private:
    virtual const DrawRecordableInfo& GetDrawRecordableInfo () const = 0;
};

} // namespace RG

#endif