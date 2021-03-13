#ifndef RG_NODE_HPP
#define RG_NODE_HPP

#include "GearsVkAPI.hpp"

#include "Noncopyable.hpp"
#include "UUID.hpp"
#include <memory>

namespace GVK {

namespace RG {

class GVK_RENDERER_API Node : public Noncopyable {
private:
    GVK::UUID uuid;

public:
    virtual ~Node () = default;

    const GVK::UUID& GetUUID () const { return uuid; }
};

} // namespace RG

} // namespace GVK

#endif
