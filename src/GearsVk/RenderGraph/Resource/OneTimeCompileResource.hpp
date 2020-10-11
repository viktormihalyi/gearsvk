#ifndef RG_ONETIMECOMPILERESOURCE_HPP
#define RG_ONETIMECOMPILERESOURCE_HPP

#include "GraphSettings.hpp"
#include "ImageResource.hpp"

namespace RG {


USING_PTR (OneTimeCompileResource);
class GEARSVK_API OneTimeCompileResource : public ImageResource {
private:
    bool compiled;

protected:
    OneTimeCompileResource ();

public:
    virtual ~OneTimeCompileResource () override = default;

    void Compile (const GraphSettings& settings) override final
    {
        if (!compiled) {
            compiled = true;
            CompileOnce (settings);
        }
    }

    virtual void CompileOnce (const GraphSettings&) = 0;
};

} // namespace RG

#endif