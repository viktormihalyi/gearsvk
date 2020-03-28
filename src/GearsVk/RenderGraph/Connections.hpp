#ifndef RENDERGRAPH_CONNECTIONS_HPP
#define RENDERGRAPH_CONNECTIONS_HPP

#include "Image.hpp"
#include <vulkan/vulkan.h>

struct InputBinding {
    const uint32_t               binding;
    VkDescriptorSetLayoutBinding descriptor;

    InputBinding (uint32_t binding, uint32_t arrayLayers)
        : binding (binding)
    {
        descriptor.binding            = binding;
        descriptor.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor.descriptorCount    = arrayLayers;
        descriptor.stageFlags         = VK_SHADER_STAGE_ALL_GRAPHICS;
        descriptor.pImmutableSamplers = nullptr;
    }

    bool operator== (uint32_t otherBinding) const { return binding == otherBinding; }
};


struct OutputBinding {
    const uint32_t          binding;
    VkAttachmentDescription attachmentDescription;
    VkAttachmentReference   attachmentReference;
    const VkFormat          format;
    const VkImageLayout     finalLayout;

    OutputBinding (uint32_t binding, VkFormat format, VkImageLayout finalLayout)
        : binding (binding)
        , attachmentDescription ({})
        , attachmentReference ({})
        , format (format)
        , finalLayout (finalLayout)
    {
        attachmentDescription.format         = format;
        attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout  = Image::INITIAL_LAYOUT;
        attachmentDescription.finalLayout    = finalLayout;

        attachmentReference.attachment = binding;
        attachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    bool operator== (uint32_t otherBinding) const { return binding == otherBinding; }
};

#endif