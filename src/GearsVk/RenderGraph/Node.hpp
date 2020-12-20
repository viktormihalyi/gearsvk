#ifndef RG_NODE_HPP
#define RG_NODE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "UUID.hpp"
#include <vector>

namespace RG {

USING_PTR (Node);
class GEARSVK_API Node : public Noncopyable {
private:
    GearsVk::UUID uuid;

public:
    virtual ~Node () = default;

    const GearsVk::UUID& GetUUID () const { return uuid; }
};

} // namespace RG

#endif