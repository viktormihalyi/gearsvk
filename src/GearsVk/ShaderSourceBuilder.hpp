#ifndef SHADERSOURCEBUILDER_HPP
#define SHADERSOURCEBUILDER_HPP

#include "Ptr.hpp"

#include <string>

class ShaderSourceBuilder {
public:
    USING_PTR_ABSTRACT (ShaderSourceBuilder);

    virtual ~ShaderSourceBuilder () = default;

    virtual std::string GetProvidedShaderSource () const = 0;
};

#endif