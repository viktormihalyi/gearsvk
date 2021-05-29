#include "BufferTransferable.hpp"

#include <cmath>

namespace GVK {

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


static ShaderType GetShaderTypeFromFormat (VkFormat format)
{
    switch (format) {
        case VK_FORMAT_R32_SFLOAT: return vec1;
        case VK_FORMAT_R32G32_SFLOAT: return vec2;
        case VK_FORMAT_R32G32B32_SFLOAT: return vec3;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return vec4;
        case VK_FORMAT_R32_UINT: return vec1;
        case VK_FORMAT_R32G32_UINT: return vec2;
        case VK_FORMAT_R32G32B32_UINT: return vec3;
        case VK_FORMAT_R32G32B32A32_UINT: return vec4;
    }

    throw std::runtime_error ("unhandled VkFormat value");
}


VertexInputInfo::VertexInputInfo (const std::vector<VkFormat>& vertexInputFormats, VkVertexInputRate inputRate)
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
    bindingDescription.inputRate = inputRate;

    bindings = {bindingDescription};
}


std::vector<VkVertexInputAttributeDescription> VertexInputInfo::GetAttributes (uint32_t firstLocation, uint32_t binding) const
{
    std::vector<VkVertexInputAttributeDescription> result = attributes;
    for (auto& a : result) {
        a.location += firstLocation;
        a.binding = binding;
    }
    return result;
}


std::vector<VkVertexInputBindingDescription> VertexInputInfo::GetBindings (uint32_t binding) const
{
    std::vector<VkVertexInputBindingDescription> result = bindings;
    for (auto& a : result) {
        a.binding = binding;
    }
    return result;
}

}
