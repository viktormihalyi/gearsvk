#ifndef DRAWRECORDABLE_HPP
#define DRAWRECORDABLE_HPP

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

#include "CommandBuffer.hpp"
#include "Ptr.hpp"

namespace GVK {

USING_PTR (VertexAttributeProvider);
class VertexAttributeProvider {
public:
    virtual ~VertexAttributeProvider () = default;

    virtual std::vector<VkVertexInputAttributeDescription> GetAttributes () const = 0;
    virtual std::vector<VkVertexInputBindingDescription>   GetBindings () const   = 0;
};


USING_PTR (PureDrawRecordable);
class PureDrawRecordable {
public:
    virtual ~PureDrawRecordable () = default;

    virtual void Record (CommandBuffer&) const = 0;
};


USING_PTR (LambdaPureDrawRecordable);
class LambdaPureDrawRecordable : private PureDrawRecordable {
public:
    using Type = std::function<void (CommandBuffer&)>;

private:
    Type callback;

public:
    LambdaPureDrawRecordable (const Type& callback)
        : callback (callback)
    {
    }

private:
    virtual void Record (CommandBuffer& c) const override
    {
        callback (c);
    }
};

USING_PTR (DrawRecordable);
class DrawRecordable : public VertexAttributeProvider, public PureDrawRecordable {
public:
    virtual ~DrawRecordable () = default;
};

} // namespace GVK

#endif