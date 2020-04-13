#ifndef DR_FULLSCREENQUAD_HPP
#define DR_FULLSCREENQUAD_HPP

#include "DrawRecordable.hpp"

class FullscreenQuad : public DrawRecordableInfoProvider {
private:
    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    VertexBufferTransferable<Vertex> vertexBuffer;
    IndexBufferTransferable          indexBuffer;

    DrawRecordableInfo::U info;

public:
    USING_PTR (FullscreenQuad);

    FullscreenQuad (const Device& device, VkQueue queue, VkCommandPool commandPool)
        : vertexBuffer (device, queue, commandPool, {ShaderTypes::Vec2f, ShaderTypes::Vec2f}, 4)
        , indexBuffer (device, queue, commandPool, 6)
    {
        vertexBuffer = {
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
    virtual const DrawRecordableInfo& GetDrawRecordableInfo () const override { return *info; }
};

#endif