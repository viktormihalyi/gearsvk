#ifndef RG_NODE_HPP
#define RG_NODE_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "Utils/Noncopyable.hpp"
#include "Utils/UUID.hpp"
#include <memory>
#include <string>

namespace RG {

class GVK_RENDERER_API Node : public Noncopyable {
private:
    GVK::UUID   uuid;
    std::string name;
    std::string debugInfo;

public:
    virtual ~Node () = default;

    const GVK::UUID& GetUUID () const { return uuid; }

    void SetName (const std::string& value) { name = value; }
    void SetDebugInfo (const std::string& value) { debugInfo = value; }

    const std::string& GetName () const { return name; }
    const std::string& GetDebugInfo () const { return debugInfo; }
};

} // namespace RG

#endif
