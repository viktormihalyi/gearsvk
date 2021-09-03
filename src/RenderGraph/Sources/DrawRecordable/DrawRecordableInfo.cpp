#include "DrawRecordableInfo.hpp"

#include "VulkanWrapper/Commands.hpp"


namespace RG {

void DrawRecordableInfo::Record (GVK::CommandBuffer& commandBuffer) const
{
    if (!vertexBuffer.empty ()) {
        std::vector<VkDeviceSize> offsets (vertexBuffer.size (), 0);
        commandBuffer.Record<GVK::CommandBindVertexBuffers> (0, static_cast<uint32_t> (vertexBuffer.size ()), vertexBuffer, offsets).SetName ("DrawRecordableInfo");
    }

    if (indexBuffer != VK_NULL_HANDLE) {
        commandBuffer.Record<GVK::CommandBindIndexBuffer> (indexBuffer, 0, VK_INDEX_TYPE_UINT16).SetName ("DrawRecordableInfo");
    }

    if (indexBuffer != VK_NULL_HANDLE) {
        commandBuffer.Record<GVK::CommandDrawIndexed> (indexCount, instanceCount, 0, 0, 0).SetName ("DrawRecordableInfo");
    } else {
        commandBuffer.Record<GVK::CommandDraw> (vertexCount, instanceCount, 0, 0).SetName ("DrawRecordableInfo");
    }
}

} // namespace RG
