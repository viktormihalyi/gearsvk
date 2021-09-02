#ifndef RENDERGRAPH_CONNECTIONBUILDER_HPP
#define RENDERGRAPH_CONNECTIONBUILDER_HPP

#include "RenderGraph/Connections.hpp"
#include "RenderGraph/Operation.hpp"
#include "RenderGraph/Resource.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <functional>
#include <memory>


namespace RG {

class OutputBuilder {
public:
    std::shared_ptr<RG::Operation>     op;
    std::shared_ptr<RG::ImageResource> target;

    std::optional<uint32_t>            binding;
    std::optional<VkFormat>            format;
    std::optional<VkImageLayout>       initialLayout;
    std::optional<VkImageLayout>       finalLayout;
    std::optional<uint32_t>            layerCount;
    std::optional<VkAttachmentLoadOp>  loadOp;
    std::optional<VkAttachmentStoreOp> storeOp;

    OutputBuilder ()
        : storeOp (VK_ATTACHMENT_STORE_OP_STORE)
    {
    }

    OutputBuilder& SetOperation (const std::shared_ptr<RG::Operation>& value)
    {
        op = value;
        return *this;
    }

    OutputBuilder& SetTarget (const std::shared_ptr<RG::ImageResource>& value)
    {
        target = value;
        SetLayerCount (target->GetLayerCount ());
        SetFormat (target->GetFormat ());
        SetInitialLayout (target->GetInitialLayout ());
        SetFinalLayout (target->GetFinalLayout ());
        return *this;
    }

    OutputBuilder& SetBinding (uint32_t value)
    {
        binding = value;
        return *this;
    }
    OutputBuilder& SetFormat (VkFormat value)
    {
        format = value;
        return *this;
    }
    OutputBuilder& SetInitialLayout (VkImageLayout value)
    {
        initialLayout = value;
        return *this;
    }
    OutputBuilder& SetFinalLayout (VkImageLayout value)
    {
        finalLayout = value;
        return *this;
    }
    OutputBuilder& SetLayerCount (uint32_t value)
    {
        layerCount = value;
        return *this;
    }

    OutputBuilder& SetLoadOp (VkAttachmentLoadOp value)
    {
        loadOp = value;
        return *this;
    }
    OutputBuilder& SetStoreOp (VkAttachmentStoreOp value)
    {
        storeOp = value;
        return *this;
    }

    OutputBuilder& SetLoad () { return SetLoadOp (VK_ATTACHMENT_LOAD_OP_LOAD); }
    OutputBuilder& SetClear () { return SetLoadOp (VK_ATTACHMENT_LOAD_OP_CLEAR); }

    [[nodiscard]] std::unique_ptr<RG::OutputBinding> BuildBinding () const
    {
        return std::make_unique<RG::OutputBinding> (
            *binding,
            [=] () -> VkFormat { return *format; },
            *initialLayout,
            *finalLayout,
            *layerCount,
            *loadOp,
            *storeOp);
    }

    [[nodiscard]] std::unique_ptr<RG::NodeConnection> Build () const
    {
        return std::make_unique<RG::NodeConnection> (op, target, BuildBinding ());
    }
};

} // namespace RG

#endif