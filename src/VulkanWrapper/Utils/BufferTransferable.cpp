#include "BufferTransferable.hpp"


struct ShaderType {
    uint32_t size;
    uint32_t alignment;
};

static ShaderType vec1 {4, 4};
static ShaderType vec2 {8, 8};
static ShaderType vec3 {12, 16};
static ShaderType vec4 {12, 16};

template<uint32_t SIZE>
static ShaderType vec1Array {4 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec2Array {8 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec3Array {12 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec4Array {16 * SIZE, 32};

class UniformBlock {
    std::vector<std::pair<ShaderType, uint32_t>> variables;

    UniformBlock (const std::vector<ShaderType>& types)
    {
        uint32_t offset = 0;
        for (auto s : types) {
            variables.emplace_back (s, offset);
            offset += std::ceil (s.size / s.alignment) * s.alignment;
        }
    }
};

static ShaderType GetShaderTypeFromFormat (VkFormat format)
{
    switch (format) {
        case VK_FORMAT_R32_SFLOAT: return vec1;
        case VK_FORMAT_R32G32_SFLOAT: return vec2;
        case VK_FORMAT_R32G32B32_SFLOAT: return vec3;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return vec4;
    }

    throw std::runtime_error ("unhandled VkFormat value");
}


static uint32_t GetAlignedBlockSize (const std::vector<VkFormat>& formats)
{
    uint32_t size = 0;
    for (auto& a : formats) {
        size += GetShaderTypeFromFormat (a).alignment;
    }
    return size;
}


VertexInputInfo::VertexInputInfo (const std::vector<VkFormat>& vertexInputFormats)
    : size (0)
{
    uint32_t location = 0;
    size              = 0;

    uint32_t attributeSize = 0;

    for (VkFormat format : vertexInputFormats) {
        VkVertexInputAttributeDescription attrib;

        attrib.binding  = 0;
        attrib.location = location;
        attrib.format   = format;
        attrib.offset   = attributeSize;
        attributes.push_back (attrib);

        ++location;
        size += GetShaderTypeFromFormat (format).size;
        attributeSize += GetShaderTypeFromFormat (format).size;
    }

    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription           = {};
    bindingDescription.binding   = 0;
    bindingDescription.stride    = size;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    bindings = {bindingDescription};
}