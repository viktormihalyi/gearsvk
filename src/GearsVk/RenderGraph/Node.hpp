#ifndef RG_NODE_HPP
#define RG_NODE_HPP

#include "GearsVkAPI.hpp"

#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "UUID.hpp"

namespace RG {

USING_PTR (Node);
class GVK_RENDERER_API Node : public Noncopyable {
private:
    GearsVk::UUID uuid;

public:
    virtual ~Node () = default;

    const GearsVk::UUID& GetUUID () const { return uuid; }
};

} // namespace RG

#endif