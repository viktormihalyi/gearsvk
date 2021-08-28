#ifndef DRAWRECORDABLE_HPP
#define DRAWRECORDABLE_HPP

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

#include "VulkanWrapper/CommandBuffer.hpp"
#include <memory>

namespace RG {

class VertexAttributeProvider {
public:
    virtual ~VertexAttributeProvider () = default;

    virtual std::vector<VkVertexInputAttributeDescription> GetAttributes () const = 0;
    virtual std::vector<VkVertexInputBindingDescription>   GetBindings () const   = 0;
};


class PureDrawRecordable {
public:
    virtual ~PureDrawRecordable () = default;

    virtual void Record (GVK::CommandBuffer&) const = 0;
};


class LambdaPureDrawRecordable : private PureDrawRecordable {
public:
    using Type = std::function<void (GVK::CommandBuffer&)>;

private:
    Type callback;

public:
    LambdaPureDrawRecordable (const Type& callback)
        : callback (callback)
    {
    }

private:
    virtual void Record (GVK::CommandBuffer& c) const override
    {
        callback (c);
    }
};

class DrawRecordable : public VertexAttributeProvider, public PureDrawRecordable {
public:
    virtual ~DrawRecordable () = default;
};

} // namespace RG

#endif