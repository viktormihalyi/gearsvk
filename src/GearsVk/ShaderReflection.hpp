#ifndef GEARS_SHADERREFLECTION_HPP
#define GEARS_SHADERREFLECTION_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>


namespace Gears {


struct UniformMember {
    std::string             name;
    std::string             type;
    uint32_t                offset;
    uint32_t                stride;
    std::optional<uint32_t> arraySize;
};


struct UniformBlock {
    std::string name;
    uint32_t    set;
    uint32_t    binding;
    uint32_t    blockSize;

    std::vector<UniformMember> members;
};

struct UniformTexture {
    std::string type;
    std::string name;
    uint32_t    set;
    uint32_t    binding;
};

struct UniformArrayInfo {
    uint32_t size;
    bool     size_is_literal;
    uint32_t stride;
};

struct UniformMatrixInfo {
    uint32_t stride;
};

struct UniformData {
private:
    struct NameType {
        std::string name;
        std::string type;
    };
    struct SetBinding {
        uint32_t set;
        uint32_t binding;
    };
    struct Array final {
        uint32_t size;
        bool     arraySizeIsLiteral;
        uint32_t stride;
    };
    struct ArrayInfo {
        std::optional<Array> arrayInfo;
    };

public:
    struct BlockMember : NameType, ArrayInfo {
        uint32_t offset;
    };

    struct Block : NameType, SetBinding {
        uint32_t                 blockSize;
        std::vector<BlockMember> members;

        VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding () const
        {
            VkDescriptorSetLayoutBinding result = {};
            result.binding                      = binding;
            result.descriptorCount              = 1;
            result.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            result.pImmutableSamplers           = nullptr;
            result.stageFlags                   = VK_SHADER_STAGE_ALL_GRAPHICS;
            return result;
        }
    };

    struct Texture : NameType, SetBinding, ArrayInfo {
    };

    struct Sampler : NameType, SetBinding, ArrayInfo {
        VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding () const
        {
            VkDescriptorSetLayoutBinding result = {};
            result.binding                      = binding;
            result.descriptorCount              = 1;
            result.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            result.pImmutableSamplers           = nullptr;
            result.stageFlags                   = VK_SHADER_STAGE_ALL_GRAPHICS;
            return result;
        }
    };

    std::vector<Block>   ubos;
    std::vector<Sampler> samplers;
};

class ShaderReflection {
private:
    UniformData data;

    std::vector<UniformBlock>   uniformData;
    std::vector<UniformTexture> textures;

public:
    ShaderReflection (const std::string& reflectionJson);
    ShaderReflection (const std::vector<uint32_t>& spirvBinary);
    ShaderReflection (const std::vector<std::vector<uint32_t>>& binaries);

    const std::vector<UniformBlock>&   GeUniformData () const { return uniformData; }
    const std::vector<UniformTexture>& GeUniformTextures () const { return textures; }

    const UniformData& Get () const { return data; }

    size_t GetCompleteUniformSize () const
    {
        size_t result = 0;
        for (const UniformBlock& u : uniformData) {
            result += u.blockSize;
        }
        return result;
    }

private:
    void ParseReflectionJson (const std::string& reflectionJson);
};

} // namespace Gears

#endif