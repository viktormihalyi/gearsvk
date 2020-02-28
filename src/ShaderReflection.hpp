#ifndef GEARS_SHADERREFLECTION_HPP
#define GEARS_SHADERREFLECTION_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>


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


class ShaderReflection {
private:
    std::vector<UniformBlock> uniformData;

public:
    ShaderReflection (const std::string& reflectionJson);
    ShaderReflection (const std::vector<uint32_t>& spirvBinary);

    const std::vector<UniformBlock>& GeUniformData () const
    {
        return uniformData;
    }

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