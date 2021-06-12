#ifndef RG_NODE_HPP
#define RG_NODE_HPP

#include "GearsVk/GearsVkAPI.hpp"

#include "Utils/Noncopyable.hpp"
#include "Utils/UUID.hpp"
#include <memory>
#include <string>

namespace GVK {

namespace RG {

class GVK_RENDERER_API Node : public Noncopyable {
private:
    GVK::UUID   uuid;
    std::string name;
    std::string description;

public:
    virtual ~Node () = default;

    const GVK::UUID& GetUUID () const { return uuid; }

    void SetName (const std::string& value) { name = value; }
    void SetDescription (const std::string& value) { description = value; }

    const std::string& GetName () const { return name; }
    const std::string& GetDescription () const { return description; }
};

} // namespace RG

} // namespace GVK

#endif
