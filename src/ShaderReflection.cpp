#include "ShaderReflection.hpp"

#include "Assert.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <spirv_reflect.hpp>

using json = nlohmann::json;

namespace Gears {

ShaderReflection::ShaderReflection (const std::string& reflectionJson)
{
    ParseReflectionJson (reflectionJson);
}


static json GetReflectionJson (const std::vector<uint32_t>& binary)
{
    spirv_cross::CompilerReflection ccc (binary);
    return ccc.compile ();
}


ShaderReflection::ShaderReflection (const std::vector<uint32_t>& spirvBinary)
{
    ParseReflectionJson (GetReflectionJson (spirvBinary));
}


ShaderReflection::ShaderReflection (const std::vector<std::vector<uint32_t>>& binaries)
{
    json unifiedReflection;
    for (const std::vector<uint32_t>& binary : binaries) {
        json n = GetReflectionJson (binary);
        unifiedReflection.insert (unifiedReflection.end (), n.begin (), n.end ());
    }
    ParseReflectionJson (unifiedReflection.dump ());
}


void ShaderReflection::ParseReflectionJson (const std::string& reflectionJsonString)
{
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

    for (const auto& t : reflectionJson["textures"]) {
        textures.push_back (UniformTexture {t["type"], t["name"], t["set"], t["binding"]});
    }

#define POPULATE_NAME_TYPE(uni, js) \
    uni.name = js["name"];          \
    uni.type = js["type"]

#define POPULATE_SET_BINDING(uni, js) \
    uni.set     = js["set"];          \
    uni.binding = js["binding"]

#define POPULATE_ARRAYINFO(uni, js)                                         \
    if (js.contains ("array")) {                                            \
        uni.arrayInfo                     = {0, true};                      \
        uni.arrayInfo->size               = js["array"][0];                 \
        uni.arrayInfo->arraySizeIsLiteral = js["array_size_is_literal"][0]; \
    }

    for (const auto& res : reflectionJson["ubos"]) {
        UniformData::Block r;
        POPULATE_NAME_TYPE (r, res);
        POPULATE_SET_BINDING (r, res);
        r.blockSize = res["block_size"];

        r.members = {};
        for (const auto& member : reflectionJson["types"][r.type]["members"]) {
            UniformData::BlockMember m;
            POPULATE_NAME_TYPE (m, member);
            POPULATE_ARRAYINFO (m, member);
            if (ERROR (m.type[0] == '_')) {
                throw std::runtime_error ("custom structs are not supported in ubos");
            }
            m.offset = member["offset"];
            r.members.push_back (m);
        }
        data.ubos.push_back (r);
    }
    /*for (const auto& res : reflectionJson["seperate_images"]) {
        UniformData::Image r;
        POPULATE_NAME_TYPE (r, res);
        POPULATE_SET_BINDING (r, res);
        POPULATE_ARRAYINFO (r, res);
        data.images.push_back (r);
    }*/
    for (const auto& res : reflectionJson["textures"]) {
        UniformData::Sampler r;
        POPULATE_NAME_TYPE (r, res);
        POPULATE_SET_BINDING (r, res);
        POPULATE_ARRAYINFO (r, res);
        data.samplers.push_back (r);
    }
}

} // namespace Gears