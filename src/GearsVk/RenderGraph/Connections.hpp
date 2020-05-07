#ifndef RENDERGRAPH_CONNECTIONS_HPP
#define RENDERGRAPH_CONNECTIONS_HPP

#include "ForFrameProvider.hpp"
#include "Image.hpp"
#include <vulkan/vulkan.h>

namespace RG {

class IInputBinding {
public:
    USING_PTR_ABSTRACT (IInputBinding);
    virtual uint32_t         GetBinding () = 0;
    virtual VkDescriptorType GetType ()    = 0;
    virtual uint32_t         GetOffset ()  = 0;
    virtual uint32_t         GetSize ()    = 0;

    virtual std::vector<VkDescriptorImageInfo>  GetImageInfos (uint32_t frameIndex) { return {}; }
    virtual std::vector<VkDescriptorBufferInfo> GetBufferInfos (uint32_t frameIndex) { return {}; }

    VkDescriptorSetLayoutBinding ToDescriptorSetLayoutBinding ()
    {
        VkDescriptorSetLayoutBinding result = {};
        result.binding                      = GetBinding ();
        result.descriptorType               = GetType ();
        result.descriptorCount              = 1;
        result.stageFlags                   = VK_SHADER_STAGE_ALL;
        result.pImmutableSamplers           = nullptr;
        return result;
    }

    void WriteToDescriptorSet (VkDevice device, VkDescriptorSet dstSet, uint32_t frameIndex)
    {
        std::vector<VkDescriptorImageInfo>  imgInfos = GetImageInfos (frameIndex);
        std::vector<VkDescriptorBufferInfo> bufInfos = GetBufferInfos (frameIndex);

        uint32_t infosSize = imgInfos.size () + bufInfos.size ();
        ASSERT (infosSize != 0);

        VkWriteDescriptorSet result = {};
        result.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        result.dstSet               = dstSet;
        result.dstBinding           = GetBinding ();
        result.dstArrayElement      = 0;
        result.descriptorType       = GetType ();
        result.descriptorCount      = infosSize;
        result.pBufferInfo          = bufInfos.data ();
        result.pImageInfo           = imgInfos.data ();
        result.pTexelBufferView     = nullptr;

        vkUpdateDescriptorSets (device, 1, &result, 0, nullptr);
    }
};


class UniformInputBinding : public IInputBinding {
public:
    USING_PTR (UniformInputBinding);

    BufferForFrameProvider& bufferProvider;
    const uint32_t          binding;
    const uint32_t          size;
    const uint32_t          offset;

    UniformInputBinding (uint32_t binding, BufferForFrameProvider& bufferProvider, uint32_t size, uint32_t offset)
        : bufferProvider (bufferProvider)
        , binding (binding)
        , size (size)
        , offset (offset)
    {
    }

    UniformInputBinding (uint32_t binding, BufferForFrameProvider& bufferProvider)
        : bufferProvider (bufferProvider)
        , binding (binding)
        , size (bufferProvider.GetBufferSize ())
        , offset (0)
    {
    }

    virtual uint32_t         GetBinding () override { return binding; }
    virtual VkDescriptorType GetType () override { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }
    virtual uint32_t         GetOffset () { return size; }
    virtual uint32_t         GetSize () { return offset; }

    virtual std::vector<VkDescriptorBufferInfo> GetBufferInfos (uint32_t frameIndex) override
    {
        VkDescriptorBufferInfo result = {};
        result.buffer                 = bufferProvider.GetBufferForFrame (frameIndex);
        result.offset                 = offset;
        result.range                  = size;
        return {result};
    }
};


class ImageInputBinding : public IInputBinding {
public:
    USING_PTR (ImageInputBinding);

    ImageViewForFrameProvider& imageViewProvider;
    const uint32_t             binding;

    ImageInputBinding (uint32_t binding, ImageViewForFrameProvider& imageViewProvider)
        : imageViewProvider (imageViewProvider)
        , binding (binding)
    {
    }

    virtual uint32_t         GetBinding () override { return binding; }
    virtual VkDescriptorType GetType () override { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }

    virtual uint32_t GetOffset () override { return 0; }
    virtual uint32_t GetSize () override { return 0; }

    virtual std::vector<VkDescriptorImageInfo> GetImageInfos (uint32_t frameIndex) override
    {
        VkDescriptorImageInfo result = {};
        result.sampler               = imageViewProvider.GetSampler ();
        result.imageView             = imageViewProvider.GetImageViewForFrame (frameIndex);
        result.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        return {result};
    }
};


struct InputBinding : public VkDescriptorSetLayoutBinding {
public:
    uint32_t offset;
    uint32_t size;

    InputBinding (uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL)
        : offset (0)
        , size (0)
    {
        this->binding            = binding;
        this->descriptorType     = type;
        this->descriptorCount    = descriptorCount;
        this->stageFlags         = stageFlags;
        this->pImmutableSamplers = nullptr;
    }
    bool operator== (const InputBinding& other) const { return binding == other.binding && descriptorType == other.descriptorType; }
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
        attachmentDescription.initialLayout  = ImageBase::INITIAL_LAYOUT;
        attachmentDescription.finalLayout    = finalLayout;

        attachmentReference.attachment = binding;
        attachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    bool operator== (uint32_t otherBinding) const { return binding == otherBinding; }
};

} // namespace RG

#endif