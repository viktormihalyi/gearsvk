#ifndef DR_FULLSCREENQUAD_HPP
#define DR_FULLSCREENQUAD_HPP

#include "DeviceExtra.hpp"
#include "DrawRecordableInfo.hpp"

#include "glmlib.hpp"

USING_PTR (FullscreenQuad);

class FullscreenQuad : public DrawRecordableInfoProvider {
private:
    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    VertexBufferTransferable<Vertex> vertexBuffer;
    IndexBufferTransferable          indexBuffer;

    DrawRecordableInfoU info;

public:
    USING_CREATE (FullscreenQuad);

    FullscreenQuad (const Device& device, VkQueue queue, VkCommandPool commandPool)
        : vertexBuffer (device, queue, commandPool, 4, {VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32_SFLOAT})
        , indexBuffer (device, queue, commandPool, 6)
    {
        vertexBuffer = std::vector<Vertex> {
            {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f)},
            {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f)},
            {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f)},
            {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f)},
        };

        indexBuffer = {0, 1, 2, 0, 3, 2};

        vertexBuffer.Flush ();
        indexBuffer.Flush ();

        info = DrawRecordableInfo::Create (1, vertexBuffer, indexBuffer);
    }

    FullscreenQuad (const DeviceExtra& device)
        : FullscreenQuad (device.device, device.graphicsQueue, device.commandPool)
    {
    }

private:
    virtual const DrawRecordableInfo& GetDrawRecordableInfo () const override
    {
        return *info;
    }
};

#endif