#include "BufferTransferable.hpp"

#include <cmath>

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


VertexInputInfo::VertexInputInfo (const std::vector<VkFormat>& vertexInputFormats, const std::optional<std::vector<std::string>>& attributeNames)
    : size (0)
    , attributeNames (attributeNames)
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


static std::string VkFormatToGLSLTypeString (VkFormat format)
{
    switch (format) {
        case VK_FORMAT_R32_SFLOAT: return "float";
        case VK_FORMAT_R32G32_SFLOAT: return "vec2";
        case VK_FORMAT_R32G32B32_SFLOAT: return "vec3";
        case VK_FORMAT_R32G32B32A32_SFLOAT: return "vec4";

        default:
            ASSERT (true);
            return "vec4";
    }
}


std::string VertexInputInfo::GetProvidedShaderSource () const
{
    if (ERROR (!attributeNames.has_value ())) {
        return "";
    }
    if (ERROR (attributeNames->size () != attributes.size ())) {
        return "";
    }

    std::string result;
    for (uint32_t i = 0; i < attributes.size (); ++i) {
        result += "layout (location = " + std::to_string (attributes[i].location) + ") in " + VkFormatToGLSLTypeString (attributes[i].format) + " " + attributeNames->at (i) + ";\n";
    }
    return result;
}
