#ifndef RENDERGRAPH_CONNECTIONS_HPP
#define RENDERGRAPH_CONNECTIONS_HPP

#include "Utils/Assert.hpp"
#include "Utils/Noncopyable.hpp"

#include "InputBindable.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <functional>
#include <vector>

namespace RG {


class IConnectionBinding : public Noncopyable {
public:
    virtual ~IConnectionBinding () override = default;

    virtual std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const { return {}; }
    virtual std::vector<VkAttachmentReference>   GetAttachmentReferences () const { return {}; }
};


class DummyIConnectionBinding : public IConnectionBinding {
public:
    virtual ~DummyIConnectionBinding () override = default;
};


class OutputBinding : public IConnectionBinding {
public:
    const uint32_t             binding;
    std::function<VkFormat ()> formatProvider;
    const VkImageLayout        initialLayout;
    const VkImageLayout        finalLayout;
    const uint32_t             layerCount;
    const VkAttachmentLoadOp   loadOp;
    const VkAttachmentStoreOp  storeOp;

    OutputBinding (uint32_t                   binding,
                   std::function<VkFormat ()> formatProvider,
                   VkImageLayout              finalLayout,
                   VkAttachmentLoadOp         loadOp,
                   VkAttachmentStoreOp        storeOp)
        : OutputBinding (binding, formatProvider, VK_IMAGE_LAYOUT_UNDEFINED, finalLayout, 1, loadOp, storeOp)
    {
    }

    OutputBinding (uint32_t                   binding,
                   std::function<VkFormat ()> formatProvider,
                   VkImageLayout              finalLayout,
                   uint32_t                   layerCount,
                   VkAttachmentLoadOp         loadOp,
                   VkAttachmentStoreOp        storeOp)
        : OutputBinding (binding, formatProvider, VK_IMAGE_LAYOUT_UNDEFINED, finalLayout, layerCount, loadOp, storeOp)
    {
    }

    OutputBinding (uint32_t                   binding,
                   std::function<VkFormat ()> formatProvider,
                   VkImageLayout              initialLayout,
                   VkImageLayout              finalLayout,
                   uint32_t                   layerCount,
                   VkAttachmentLoadOp         loadOp,
                   VkAttachmentStoreOp        storeOp)
        : binding (binding)
        , formatProvider (formatProvider)
        , initialLayout (initialLayout)
        , finalLayout (finalLayout)
        , layerCount (layerCount)
        , loadOp (loadOp)
        , storeOp (storeOp)
    {
        GVK_ASSERT (layerCount > 0 && layerCount < 1024);
        GVK_ERROR (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD && initialLayout == VK_IMAGE_LAYOUT_UNDEFINED);
    }

    virtual std::vector<VkAttachmentReference> GetAttachmentReferences () const override
    {
        std::vector<VkAttachmentReference> result;
        for (uint32_t bindingIndex = binding; bindingIndex < binding + layerCount; ++bindingIndex) {
            // layout is a VkImageLayout value specifying the layout the attachment uses during the subpass.
            // automatic!!!
            VkAttachmentReference ref = {};
            ref.attachment            = bindingIndex;
            ref.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            result.push_back (ref);
        }
        return result;
    }

    virtual std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const override
    {
        
        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.flags                   = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
        attachmentDescription.format                  = formatProvider ();
        attachmentDescription.samples                 = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp                  = loadOp;
        attachmentDescription.storeOp                 = storeOp;
        attachmentDescription.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // initialLayout is the layout the attachment image subresource will be in when a render pass instance begins.
        attachmentDescription.initialLayout = initialLayout; // TODO why not color attachment ???

        // finalLayout is the layout the attachment image subresource will be transitioned to when a render pass instance ends.
        // automatic!!!
        attachmentDescription.finalLayout = finalLayout;

        return std::vector<VkAttachmentDescription> (layerCount, attachmentDescription);
    }

    bool operator== (uint32_t otherBinding) const { return binding == otherBinding; }

};

} // namespace RG

#endif