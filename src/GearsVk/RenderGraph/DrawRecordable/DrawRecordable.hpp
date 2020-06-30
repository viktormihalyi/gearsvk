#ifndef DRAWRECORDABLE_HPP
#define DRAWRECORDABLE_HPP

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

#include "Ptr.hpp"

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

    virtual void Record (VkCommandBuffer) const = 0;
};

USING_PTR (LambdaPureDrawRecordable);
class LambdaPureDrawRecordable : private PureDrawRecordable {
public:
    using Type = std::function<void (VkCommandBuffer)>;

private:
    Type callback;

public:
    USING_CREATE (LambdaPureDrawRecordable);

    LambdaPureDrawRecordable (const Type& callback)
        : callback (callback)
    {
    }

private:
    virtual void Record (VkCommandBuffer c) const override
    {
        callback (c);
    }
};

USING_PTR (DrawRecordable);
class DrawRecordable : public VertexAttributeProvider, public PureDrawRecordable {
public:
    virtual ~DrawRecordable () = default;
};

#endif