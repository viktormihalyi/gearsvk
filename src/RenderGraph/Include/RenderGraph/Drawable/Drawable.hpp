#ifndef DRAWRECORDABLE_HPP
#define DRAWRECORDABLE_HPP

#include "RenderGraph/RenderGraphAPI.hpp"


namespace GVK {
class CommandBuffer;
}


namespace RG {

class GVK_RENDERER_API Drawable {
public:
    virtual ~Drawable ();

    virtual void Record (GVK::CommandBuffer&) const = 0;
};

} // namespace RG

#endif