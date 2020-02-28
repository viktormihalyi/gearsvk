#include "ShaderReflection.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <spirv_reflect.hpp>


namespace Gears {

ShaderReflection::ShaderReflection (const std::string& reflectionJson)
{
    ParseReflectionJson (reflectionJson);
}


ShaderReflection::ShaderReflection (const std::vector<uint32_t>& spirvBinary)
{
    spirv_cross::CompilerReflection ccc (spirvBinary);
    ParseReflectionJson (ccc.compile ());
}


void ShaderReflection::ParseReflectionJson (const std::string& reflectionJsonString)
{
    using json = nlohmann::json;

    std::cout << reflectionJsonString << std::endl;

    json reflectionJson = json::parse (reflectionJsonString);
    for (const auto& ubo : reflectionJson["ubos"]) {
        UniformBlock uniformBlock;
        uniformBlock.name      = ubo["name"];
        uniformBlock.set       = ubo["set"];
        uniformBlock.binding   = ubo["binding"];
        uniformBlock.blockSize = ubo["block_size"];

        for (const auto& member : reflectionJson["types"][ubo["type"].get<std::string> ()]["members"]) {
            UniformMember uniformMember;
            uniformMember.name   = member["name"];
            uniformMember.type   = member["type"];
            uniformMember.offset = member["offset"];
            uniformMember.stride = member.contains ("matrix_stride") ? member["matrix_stride"] : 0;
            if (member.contains ("array")) {
                uniformMember.arraySize = member["array"][0].get<int> ();
            }
            uniformBlock.members.push_back (uniformMember);
        }

        uniformData.push_back (uniformBlock);
    }
}

} // namespace Gears