#ifndef RENDERGRAPH_CONNECTIONS_HPP
#define RENDERGRAPH_CONNECTIONS_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"

#include "InputBindable.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <functional>
#include <vector>

namespace GVK {

namespace RG {

class IConnectionBindingVisitor;

USING_PTR (IConnectionBinding);
class IConnectionBinding : public Noncopyable {
public:
    ~IConnectionBinding ()                          = default;
    virtual void Visit (IConnectionBindingVisitor&) = 0;

    virtual uint32_t GetLayerCount () const = 0;

    virtual void WriteToDescriptorSet (VkDevice device, VkDescriptorSet dstSet, uint32_t frameIndex) const {}

    virtual std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const { return {}; }
    virtual std::vector<VkAttachmentReference>   GetAttachmentReferences () const { return {}; }
};

class UniformInputBinding;
class ImageInputBinding;
class OutputBinding;


USING_PTR (IConnectionBindingVisitor);
class IConnectionBindingVisitor : public Noncopyable {
public:
    virtual void Visit (UniformInputBinding& binding) = 0;
    virtual void Visit (ImageInputBinding& binding)   = 0;
    virtual void Visit (OutputBinding& binding)       = 0;
};

USING_PTR (IConnectionBindingVisitorFn);
class IConnectionBindingVisitorFn : public IConnectionBindingVisitor {
private:
    template<typename T>
    using VisitorFn = std::function<void (T&)>;

    VisitorFn<UniformInputBinding> b1;
    VisitorFn<ImageInputBinding>   b2;
    VisitorFn<OutputBinding>       b3;

public:
    IConnectionBindingVisitorFn (const VisitorFn<UniformInputBinding>& b1,
                                 const VisitorFn<ImageInputBinding>&   b2,
                                 const VisitorFn<OutputBinding>&       b3)
        : b1 (b1)
        , b2 (b2)
        , b3 (b3)
    {
    }

    virtual void Visit (UniformInputBinding& binding) override { b1 (binding); }
    virtual void Visit (ImageInputBinding& binding) override { b2 (binding); }
    virtual void Visit (OutputBinding& binding) override { b3 (binding); }
};

USING_PTR (InputBinding);
class InputBinding : public IConnectionBinding {
public:
    virtual ~InputBinding () = default;

    virtual uint32_t           GetBinding () const = 0;
    virtual VkDescriptorType   GetType () const    = 0;
    virtual uint32_t           GetOffset () const  = 0;
    virtual uint32_t           GetSize () const    = 0;
    virtual VkShaderStageFlags GetStages () const  = 0;
    virtual uint32_t           GetLayerCount () const override { return 1; }

    virtual std::vector<VkDescriptorImageInfo>  GetImageInfos (uint32_t) const { return {}; }
    virtual std::vector<VkDescriptorBufferInfo> GetBufferInfos (uint32_t) const { return {}; }

    VkDescriptorSetLayoutBinding ToDescriptorSetLayoutBinding () const
    {
        VkDescriptorSetLayoutBinding result = {};
        result.binding                      = GetBinding ();
        result.descriptorType               = GetType ();
        result.descriptorCount              = GetLayerCount ();
        result.stageFlags                   = GetStages ();
        result.pImmutableSamplers           = nullptr;
        return result;
    }

    virtual void WriteToDescriptorSet (VkDevice device, VkDescriptorSet dstSet, uint32_t frameIndex) const override
    {
        const std::vector<VkDescriptorImageInfo>  imgInfos = GetImageInfos (frameIndex);
        const std::vector<VkDescriptorBufferInfo> bufInfos = GetBufferInfos (frameIndex);

        size_t infosSize = imgInfos.size () + bufInfos.size ();
        GVK_ASSERT (infosSize != 0);

        VkWriteDescriptorSet result = {};
        result.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        result.dstSet               = dstSet;
        result.dstBinding           = GetBinding ();
        result.dstArrayElement      = 0;
        result.descriptorType       = GetType ();
        result.descriptorCount      = static_cast<uint32_t> (infosSize);
        result.pBufferInfo          = bufInfos.empty () ? nullptr : bufInfos.data ();
        result.pImageInfo           = imgInfos.empty () ? nullptr : imgInfos.data ();
        result.pTexelBufferView     = nullptr;

        vkUpdateDescriptorSets (device, 1, &result, 0, nullptr);
    }
};


USING_PTR (UniformInputBinding);
class UniformInputBinding : public InputBinding {
public:
    InputBufferBindable&     bufferProvider;
    const uint32_t           binding;
    const uint32_t           size;
    const uint32_t           offset;
    const VkShaderStageFlags stages;

    UniformInputBinding (uint32_t binding, InputBufferBindable& bufferProvider, uint32_t size, uint32_t offset, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL)
        : bufferProvider (bufferProvider)
        , binding (binding)
        , size (size)
        , offset (offset)
        , stages (stages)
    {
    }

    UniformInputBinding (uint32_t binding, InputBufferBindable& bufferProvider, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL)
        : bufferProvider (bufferProvider)
        , binding (binding)
        , size (bufferProvider.GetBufferSize ())
        , offset (0)
        , stages (stages)
    {
    }

    virtual uint32_t           GetBinding () const override { return binding; }
    virtual VkDescriptorType   GetType () const override { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }
    virtual uint32_t           GetOffset () const override { return size; }
    virtual uint32_t           GetSize () const override { return offset; }
    virtual VkShaderStageFlags GetStages () const override { return stages; }

    virtual std::vector<VkDescriptorBufferInfo> GetBufferInfos (uint32_t frameIndex) const override
    {
        VkDescriptorBufferInfo result = {};
        result.buffer                 = bufferProvider.GetBufferForFrame (frameIndex);
        result.offset                 = offset;
        result.range                  = size;
        return { result };
    }

    virtual void Visit (IConnectionBindingVisitor& visitor) override
    {
        visitor.Visit (*this);
    }
};


USING_PTR (ImageInputBinding);
class ImageInputBinding : public InputBinding {
public:
    InputImageBindable&      imageViewProvider;
    const uint32_t           binding;
    const uint32_t           layerCount;
    const VkShaderStageFlags stages;

    ImageInputBinding (uint32_t binding, InputImageBindable& imageViewProvider, uint32_t layerCount = 1, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL)
        : imageViewProvider (imageViewProvider)
        , binding (binding)
        , layerCount (layerCount)
        , stages (stages)
    {
    }

    virtual uint32_t           GetBinding () const override { return binding; }
    virtual VkDescriptorType   GetType () const override { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }
    virtual uint32_t           GetOffset () const override { return 0; }
    virtual uint32_t           GetSize () const override { return 0; }
    virtual VkShaderStageFlags GetStages () const override { return stages; }
    virtual uint32_t           GetLayerCount () const override { return layerCount; }

    virtual std::vector<VkDescriptorImageInfo> GetImageInfos (uint32_t frameIndex) const override
    {
        std::vector<VkDescriptorImageInfo> result;
        for (uint32_t imageIndex = 0; imageIndex < layerCount; ++imageIndex) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.sampler               = imageViewProvider.GetSampler ();
            imageInfo.imageView             = imageViewProvider.GetImageViewForFrame (frameIndex, imageIndex);
            imageInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            result.push_back (imageInfo);
        }
        return result;
    }

    std::vector<VkImageView> GetImageViewsForFrame (uint32_t frameIndex) const
    {
        std::vector<VkImageView> result;
        for (uint32_t imageIndex = 0; imageIndex < layerCount; ++imageIndex) {
            result.push_back (imageViewProvider.GetImageViewForFrame (frameIndex, imageIndex));
        }
        return result;
    }

    virtual void Visit (IConnectionBindingVisitor& visitor) override
    {
        visitor.Visit (*this);
    }
};


USING_PTR (OutputBinding);
class OutputBinding : public IConnectionBinding {
public:
    const uint32_t             binding;
    std::function<VkFormat ()> formatProvider;
    const VkImageLayout        finalLayout;
    const uint32_t             layerCount;
    const VkAttachmentLoadOp   loadOp;
    const VkAttachmentStoreOp  storeOp;

    OutputBinding (uint32_t                   binding,
                   std::function<VkFormat ()> formatProvider,
                   VkImageLayout              finalLayout,
                   VkAttachmentLoadOp         loadOp,
                   VkAttachmentStoreOp        storeOp)
        : OutputBinding (binding, formatProvider, finalLayout, 1, loadOp, storeOp)
    {
    }

    OutputBinding (uint32_t                   binding,
                   std::function<VkFormat ()> formatProvider,
                   VkImageLayout              finalLayout,
                   uint32_t                   layerCount,
                   VkAttachmentLoadOp         loadOp,
                   VkAttachmentStoreOp        storeOp)
        : binding (binding)
        , formatProvider (formatProvider)
        , finalLayout (finalLayout)
        , layerCount (layerCount)
        , loadOp (loadOp)
        , storeOp (storeOp)
    {
    }

    virtual uint32_t GetLayerCount () const override { return layerCount; }

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
        attachmentDescription.format                  = formatProvider ();
        attachmentDescription.samples                 = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp                  = loadOp;
        attachmentDescription.storeOp                 = storeOp;
        attachmentDescription.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // initialLayout is the layout the attachment image subresource will be in when a render pass instance begins.
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO why not color attachment ???

        // finalLayout is the layout the attachment image subresource will be transitioned to when a render pass instance ends.
        // automatic!!!
        attachmentDescription.finalLayout = finalLayout;

        return std::vector<VkAttachmentDescription> (layerCount, attachmentDescription);
    }

    bool operator== (uint32_t otherBinding) const { return binding == otherBinding; }

    virtual void Visit (IConnectionBindingVisitor& visitor) override
    {
        visitor.Visit (*this);
    }
};

} // namespace RG

} // namespace GVK

#endif