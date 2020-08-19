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

    FullscreenQuad (const DeviceExtra& device)
        : vertexBuffer (device, 4, {VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32_SFLOAT}, VK_VERTEX_INPUT_RATE_VERTEX)
        , indexBuffer (device, 6)
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

private:
    virtual const DrawRecordableInfo& GetDrawRecordableInfo () const override
    {
        return *info;
    }
};

#endif