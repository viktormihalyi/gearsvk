#include <vulkan/vulkan.h>

#include "Ptr.hpp"
#include "ShaderPipeline.hpp"
#include "Utils.hpp"
#include "VulkanWrapper.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData)
{
    std::cout << "validation layer: " << pCallbackData->pMessageIdName << ": " << pCallbackData->pMessage << std::endl
              << std::endl;
    //std::cout << "validation layer" << std::endl;
    return VK_FALSE;
}

struct BufferImage {
    Image::U        image;
    Buffer::U       buffer;
    DeviceMemory::U memory;
};


struct BufferMemory {
    Buffer::U       buffer;
    DeviceMemory::U memory;
    uint32_t        bufferSize;
    uint32_t        allocatedSize;
};

uint32_t FindMemoryType (VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties = {};
    vkGetPhysicalDeviceMemoryProperties (physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error ("failed to find suitable memory type!");
}


DeviceMemory AllocateImageMemory (VkPhysicalDevice physicalDevice, VkDevice device, VkImage image, VkMemoryPropertyFlags propertyFlags)
{
    VkMemoryRequirements memRequirements = {};
    vkGetImageMemoryRequirements (device, image, &memRequirements);
    uint32_t memoryTypeIndex = FindMemoryType (physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

    return DeviceMemory (device, memRequirements.size, memoryTypeIndex);
}

DeviceMemory AllocateBufferMemory (VkPhysicalDevice physicalDevice, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags propertyFlags)
{
    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements (device, buffer, &memRequirements);
    uint32_t memoryTypeIndex = FindMemoryType (physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

    return DeviceMemory (device, memRequirements.size, memoryTypeIndex);
}

void TransitionImageLayout (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    SingleTimeCommand commandBuffer (device, commandPool, graphicsQueue);
    image.CmdTransitionImageLayout (commandBuffer, oldLayout, newLayout);
}


auto acceptAnything    = [] (const std::vector<VkQueueFamilyProperties>&) { return 0; };
auto acceptGraphicsBit = [] (const std::vector<VkQueueFamilyProperties>& props) -> std::optional<uint32_t> {
    uint32_t i = 0;
    for (const auto& p : props) {
        if (p.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return i;
        }
        ++i;
    }
    return std::nullopt;
};

class TestEnvironment {
public:
    Instance            instance;
    DebugUtilsMessenger messenger;
    PhysicalDevice      physicalDevice;

    TestEnvironment ()
        : instance ({VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, {"VK_LAYER_KHRONOS_validation"})
        , messenger (instance, debugCallback)
        , physicalDevice (instance, {}, {acceptGraphicsBit, acceptAnything, acceptAnything, acceptAnything})
    {
    }
};

class TestCase {
public:
    Device      device;
    Queue       graphicsQueue;
    CommandPool commandPool;

    TestCase (const TestEnvironment& env)
        : device (env.physicalDevice, *env.physicalDevice.queueFamilies.graphics, {})
        , graphicsQueue (device, *env.physicalDevice.queueFamilies.graphics)
        , commandPool (device, *env.physicalDevice.queueFamilies.graphics)
    {
    }
};

void CopyBufferToImage (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    SingleTimeCommand commandBuffer (device, commandPool, graphicsQueue);

    VkBufferImageCopy region               = {};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {width, height, 1};

    vkCmdCopyBufferToImage (
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
}

struct AllocatedImage {
    Image::U        image;
    DeviceMemory::U memory;

    AllocatedImage (const Device& device, Image::U&& image, VkMemoryPropertyFlags memoryPropertyFlags)
        : image (std::move (image))
        , memory (DeviceMemory::Create (device, device.GetImageAllocateInfo (*this->image, memoryPropertyFlags)))
    {
        vkBindImageMemory (device, *this->image, *memory, 0);
    }
};

struct AllocatedBuffer {
    Buffer::U       buffer;
    DeviceMemory::U memory;

    AllocatedBuffer (const Device& device, Buffer::U&& buffer, VkMemoryPropertyFlags memoryPropertyFlags)
        : buffer (std::move (buffer))
        , memory (DeviceMemory::Create (device, device.GetBufferAllocateInfo (*this->buffer, memoryPropertyFlags)))
    {
        vkBindBufferMemory (device, *this->buffer, *memory, 0);
    }
};


AllocatedImage CreateImage (const Device& device, uint32_t width, uint32_t height, VkQueue queue, VkCommandPool commandPool)
{
    AllocatedImage result (device, Image::Create (device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), DeviceMemory::GPU);

    TransitionImageLayout (device, queue, commandPool, *result.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    {
        AllocatedBuffer stagingMemory (device, Buffer::Create (device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), DeviceMemory::CPU);
        {
            MemoryMapping                       bm (device, *stagingMemory.memory, 0, width * height * 4);
            std::vector<std::array<uint8_t, 4>> pixels (width * height);
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < width; ++x) {
                    pixels[y * width + x] = {1, 1, 127, 127};
                }
            }

            bm.Copy (pixels);
        }
        CopyBufferToImage (device, queue, commandPool, *stagingMemory.buffer, *result.image, width, height);
    }
    TransitionImageLayout (device, queue, commandPool, *result.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    return std::move (result);
}


struct InputTexture : public Noncopyable {
    AllocatedImage               image;
    ImageView::U                 imageView;
    Sampler::U                   sampler;
    uint32_t                     binding;
    VkDescriptorSetLayoutBinding descriptor;

    USING_PTR (InputTexture);

    InputTexture (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t width, uint32_t height, uint32_t binding)
        : image (CreateImage (device, width, height, queue, commandPool))
        , imageView (ImageView::Create (device, *image.image))
        , sampler (Sampler::Create (device))
        , binding (binding)
        , descriptor ({})
    {
        descriptor.binding            = binding;
        descriptor.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor.descriptorCount    = 1;
        descriptor.stageFlags         = VK_SHADER_STAGE_ALL_GRAPHICS;
        descriptor.pImmutableSamplers = nullptr;
    }
};

struct InputTextureStore {
    std::vector<InputTexture::U> textures;

    std::vector<VkDescriptorSetLayoutBinding> GetLayoutBindings () const
    {
        std::vector<VkDescriptorSetLayoutBinding> result;
        for (const auto& t : textures) {
            result.push_back (t->descriptor);
        }
        return result;
    }
};

struct OutputTexture : public Noncopyable {
    AllocatedImage          image;
    ImageView::U            imageView;
    uint32_t                binding;
    VkAttachmentDescription attachmentDescription;
    VkAttachmentReference   attachmentReference;

    USING_PTR (OutputTexture);

    OutputTexture (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t width, uint32_t height, uint32_t binding)
        : image (device, Image::Create (device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), DeviceMemory::GPU)
        , imageView (ImageView::Create (device, *image.image))
        , binding (binding)
        , attachmentDescription ({})
        , attachmentReference ({})
    {
        attachmentDescription.format         = VK_FORMAT_R8G8B8A8_SRGB;
        attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        attachmentReference.attachment = binding;
        attachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
};


struct OutputTextureStore {
    std::vector<OutputTexture::U> textures;

    std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const
    {
        std::vector<VkAttachmentDescription> result;
        for (const auto& t : textures) {
            result.push_back (t->attachmentDescription);
        }
        return result;
    }

    std::vector<VkAttachmentReference> GetAttachmentReferences () const
    {
        std::vector<VkAttachmentReference> result;
        for (const auto& t : textures) {
            result.push_back (t->attachmentReference);
        }
        return result;
    }

    std::vector<VkImageView> GetImageViews () const
    {
        std::vector<VkImageView> result;
        for (const auto& t : textures) {
            result.push_back (*t->imageView);
        }
        return result;
    }
};


std::thread SaveImageToFileAsync (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image& image, const std::string& filePath)
{
    AllocatedImage dst (device, Image::Create (device, image.GetWidth (), image.GetHeight (), image.GetFormat (), VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT), DeviceMemory::CPU);
    TransitionImageLayout (device, queue, commandPool, *dst.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    {
        SingleTimeCommand single (device, commandPool, queue);

        VkImageCopy imageCopyRegion               = {};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width              = image.GetWidth ();
        imageCopyRegion.extent.height             = image.GetHeight ();
        imageCopyRegion.extent.depth              = 1;

        vkCmdCopyImage (
            single,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            *dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    std::vector<uint8_t> mapped (512 * 512 * 4);

    {
        MemoryMapping mapping (device, *dst.memory, 0, 512 * 512 * 4);
        memcpy (mapped.data (), mapping.Get (), 512 * 512 * 4);
    }


    const uint32_t width  = image.GetWidth ();
    const uint32_t height = image.GetHeight ();

    return std::thread ([=] () {
        stbi_write_png (filePath.c_str (), width, height, 4, mapped.data (), width * 4);
    });
};

namespace RenderGraph {


struct InputBinding {
    const uint32_t               binding;
    VkDescriptorSetLayoutBinding descriptor;

    InputBinding (uint32_t binding)
        : binding (binding)
    {
        descriptor.binding            = binding;
        descriptor.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor.descriptorCount    = 1;
        descriptor.stageFlags         = VK_SHADER_STAGE_ALL_GRAPHICS;
        descriptor.pImmutableSamplers = nullptr;
    }

    bool operator== (const InputBinding& other) const
    {
        return binding == other.binding;
    }
};

struct OutputBinding {
    uint32_t                binding;
    VkAttachmentDescription attachmentDescription;
    VkAttachmentReference   attachmentReference;

    OutputBinding (uint32_t binding)
        : binding (binding)
        , attachmentDescription ({})
        , attachmentReference ({})
    {
        attachmentDescription.format         = VK_FORMAT_R8G8B8A8_SRGB;
        attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_GENERAL; // TODO
        attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_GENERAL; // TODO

        attachmentReference.attachment = binding;
        attachmentReference.layout     = VK_IMAGE_LAYOUT_GENERAL; // TODO
    }

    bool operator== (const InputBinding& other) const
    {
        return binding == other.binding;
    }
};


class Resource : public Noncopyable {
public:
    AllocatedImage image;
    ImageView::U   imageView;
    Sampler::U     sampler;

    USING_PTR (Resource);

    Resource (const Device& device, VkQueue queue, VkCommandPool commandPool)
        : image (device, Image::Create (device, 512, 512, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT), DeviceMemory::GPU)
        , imageView (ImageView::Create (device, *image.image, image.image->GetFormat ()))
        , sampler (Sampler::Create (device))
    {
        TransitionImageLayout (device, queue, commandPool, *image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    }

    virtual ~Resource () {}
};


struct Operation : public Noncopyable {
    USING_PTR_ABSTRACT (Operation);

    std::vector<Resource::Ref> inputs;
    std::vector<Resource::Ref> outputs;

    std::vector<InputBinding>  inputBindings;
    std::vector<OutputBinding> outputBindings;

    virtual ~Operation () {}

    virtual void Compile ()                              = 0;
    virtual void Execute (VkCommandBuffer commandBuffer) = 0;
    virtual void Bind (VkCommandBuffer commandBuffer)    = 0;

    void AddInput (uint32_t binding, const Resource::Ref& res)
    {
        ASSERT (std::find (inputBindings.begin (), inputBindings.end (), binding) == inputBindings.end ());

        inputs.push_back (res);
        inputBindings.push_back (binding);
    }

    void AddOutput (uint32_t binding, const Resource::Ref& res)
    {
        ASSERT (std::find (outputBindings.begin (), outputBindings.end (), binding) == outputBindings.end ());

        outputs.push_back (res);
        outputBindings.push_back (binding);
    }
};


struct PresentOperation final : public Operation {
};


struct RenderOperation final : public Operation {
    USING_PTR (RenderOperation);

    const VkDevice device;

    DescriptorPool::U      descriptorPool;
    DescriptorSet::U       descriptorSet;
    DescriptorSetLayout::U descriptorSetLayout;

    RenderOperation (VkDevice device)
        : device (device)
    {
    }

    virtual void Compile () override
    {
        if (inputBindings.empty ()) {
            return;
        }

        std::vector<VkDescriptorSetLayoutBinding> layout;
        for (auto& inputBinding : inputBindings) {
            layout.push_back (inputBinding.descriptor);
        }

        descriptorSetLayout = DescriptorSetLayout::Create (device, layout);
        descriptorPool      = DescriptorPool::Create (device, 0, inputBindings.size (), 1);
        descriptorSet       = DescriptorSet::Create (device, *descriptorPool, *descriptorSetLayout);

        for (uint32_t i = 0; i < inputs.size (); ++i) {
            Resource& r = inputs[i];
            descriptorSet->WriteOneImageInfo (
                inputBindings[i].binding,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                {*r.sampler, *r.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        }
    }

    virtual void Execute (VkCommandBuffer commandBuffer) override {}
    virtual void Bind (VkCommandBuffer commandBuffer) override {}
};

class Graph final : public Noncopyable {
public:
    USING_PTR (Graph);

    std::vector<Resource::U>  resources;
    std::vector<Operation::U> operations;

    CommandBuffer commandBuffer;

    Graph (VkDevice device, VkCommandPool commandPool)
        : commandBuffer (device, commandPool)
    {
    }

    Resource::Ref CreateResource (Resource::U&& resource)
    {
        resources.push_back (std::move (resource));
        return *resources[resources.size () - 1];
    }

    Operation::Ref CreateOperation (Operation::U&& resource)
    {
        operations.push_back (std::move (resource));
        return *operations[operations.size () - 1];
    }

    void Compile ()
    {
        for (auto& op : operations) {
            op->Compile ();
        }

        commandBuffer.Begin ();

        for (auto& op : operations) {
            op->Bind (commandBuffer);
            op->Execute (commandBuffer);
        }

        commandBuffer.End ();
    }
};
} // namespace RenderGraph


int main ()
{
    try {
        auto acceptAnything = [] (const std::vector<VkQueueFamilyProperties>&) { return 0; };
        auto acceptWithFlag = [] (VkQueueFlagBits flagbits) {
            return [&] (const std::vector<VkQueueFamilyProperties>& props) -> std::optional<uint32_t> {
                uint32_t i = 0;
                for (const auto& p : props) {
                    if (p.queueFlags & flagbits) {
                        return i;
                    }
                    ++i;
                }
                return std::nullopt;
            };
        };

        const std::vector<const char*> extensions       = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
        const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

        Instance            instance (extensions, validationLayers);
        DebugUtilsMessenger messenger (instance, debugCallback);
        PhysicalDevice      physicalDevice (instance, {}, {acceptWithFlag (VK_QUEUE_GRAPHICS_BIT), acceptAnything, acceptAnything, acceptAnything});
        Device              device (physicalDevice, *physicalDevice.queueFamilies.graphics, {});
        Queue               graphicsQueue (device, *physicalDevice.queueFamilies.graphics);
        CommandPool         commandPool (device, *physicalDevice.queueFamilies.graphics);

        using namespace RenderGraph;
        {
            Graph graph (device, commandPool);

            Resource::Ref depthBuffer    = graph.CreateResource (Resource::Create (device, graphicsQueue, commandPool));
            Resource::Ref depthBuffer2   = graph.CreateResource (Resource::Create (device, graphicsQueue, commandPool));
            Resource::Ref gbuffer1       = graph.CreateResource (Resource::Create (device, graphicsQueue, commandPool));
            Resource::Ref gbuffer2       = graph.CreateResource (Resource::Create (device, graphicsQueue, commandPool));
            Resource::Ref gbuffer3       = graph.CreateResource (Resource::Create (device, graphicsQueue, commandPool));
            Resource::Ref debugOutput    = graph.CreateResource (Resource::Create (device, graphicsQueue, commandPool));
            Resource::Ref lightingBuffer = graph.CreateResource (Resource::Create (device, graphicsQueue, commandPool));
            Resource::Ref finalTarget    = graph.CreateResource (Resource::Create (device, graphicsQueue, commandPool));

            Operation::Ref depthPass   = graph.CreateOperation (RenderOperation::Create (device));
            Operation::Ref gbufferPass = graph.CreateOperation (RenderOperation::Create (device));
            Operation::Ref debugView   = graph.CreateOperation (RenderOperation::Create (device));
            Operation::Ref move        = graph.CreateOperation (RenderOperation::Create (device));
            Operation::Ref lighting    = graph.CreateOperation (RenderOperation::Create (device));
            Operation::Ref post        = graph.CreateOperation (RenderOperation::Create (device));
            Operation::Ref present     = graph.CreateOperation (RenderOperation::Create (device));

            depthPass.get ().AddOutput (0, depthBuffer);

            gbufferPass.get ().AddInput (0, depthBuffer);
            gbufferPass.get ().AddOutput (1, depthBuffer2);
            gbufferPass.get ().AddOutput (2, gbuffer1);
            gbufferPass.get ().AddOutput (3, gbuffer2);
            gbufferPass.get ().AddOutput (4, gbuffer3);

            debugView.get ().AddInput (0, gbuffer3);
            debugView.get ().AddOutput (0, debugOutput);

            lighting.get ().AddInput (0, depthBuffer);
            lighting.get ().AddInput (1, gbuffer1);
            lighting.get ().AddInput (2, gbuffer2);
            lighting.get ().AddInput (3, gbuffer3);
            lighting.get ().AddOutput (0, lightingBuffer);

            post.get ().AddInput (0, lightingBuffer);
            post.get ().AddOutput (0, finalTarget);

            move.get ().AddInput (0, debugOutput);
            move.get ().AddOutput (0, finalTarget);

            present.get ().AddInput (0, finalTarget);

            graph.Compile ();
        }

        InputTextureStore inputs;
        inputs.textures.push_back (InputTexture::Create (device, graphicsQueue, commandPool, 512, 512, 0));
        inputs.textures.push_back (InputTexture::Create (device, graphicsQueue, commandPool, 512, 512, 1));

        OutputTextureStore outputs;
        outputs.textures.push_back (OutputTexture::Create (device, graphicsQueue, commandPool, 512, 512, 0));
        outputs.textures.push_back (OutputTexture::Create (device, graphicsQueue, commandPool, 512, 512, 1));

        DescriptorSetLayout descLayout (device, inputs.GetLayoutBindings ());
        DescriptorPool      descPool (device, 0, inputs.textures.size (), 1);
        DescriptorSet       descSet (device, descPool, descLayout);
        for (const auto& t : inputs.textures) {
            descSet.WriteOneImageInfo (t->binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, {*t->sampler, *t->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        }


        auto                 refs    = outputs.GetAttachmentReferences ();
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = outputs.textures.size ();
        subpass.pColorAttachments    = refs.data ();

        VkSubpassDependency dependency = {};
        dependency.srcSubpass          = 0;
        dependency.dstSubpass          = 0;
        dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask       = 0;
        dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount        = outputs.textures.size ();
        renderPassInfo.pAttachments           = outputs.GetAttachmentDescriptions ().data ();
        renderPassInfo.subpassCount           = 1;
        renderPassInfo.pSubpasses             = &subpass;
        renderPassInfo.dependencyCount        = 1;
        renderPassInfo.pDependencies          = &dependency;

        ShaderPipeline shaders;

        std::thread s1 ([&] () {
            shaders.vertexShader = ShaderModule::CreateFromSource (device, PROJECT_ROOT / "shaders" / "test.vert");
        });
        std::thread s2 ([&] () {
            shaders.fragmentShader = ShaderModule::CreateFromSource (device, PROJECT_ROOT / "shaders" / "test.frag");
        });

        s1.join ();
        s2.join ();

        shaders.Compile (device, 512, 512, descLayout, outputs.GetAttachmentReferences (), outputs.GetAttachmentDescriptions ());


        VkClearValue              clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        std::vector<VkClearValue> clearValues (outputs.textures.size (), clearColor);

        Framebuffer framebuffer (device, *shaders.renderPass, outputs.GetImageViews (), 512, 512);

        {
            SingleTimeCommand single (device, commandPool, graphicsQueue);

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass            = *shaders.renderPass;
            renderPassBeginInfo.framebuffer           = framebuffer;
            renderPassBeginInfo.renderArea.offset     = {0, 0};
            renderPassBeginInfo.renderArea.extent     = {512, 512};
            renderPassBeginInfo.clearValueCount       = clearValues.size ();
            renderPassBeginInfo.pClearValues          = clearValues.data ();

            vkCmdBeginRenderPass (single, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkDescriptorSet dsHandle = descSet;

            vkCmdBindPipeline (single, VK_PIPELINE_BIND_POINT_GRAPHICS, *shaders.pipeline);
            vkCmdBindDescriptorSets (single, VK_PIPELINE_BIND_POINT_GRAPHICS, *shaders.pipelineLayout, 0,
                                     1, &dsHandle,
                                     0, nullptr);
            vkCmdDraw (single, 3, 1, 0, 0);
            vkCmdEndRenderPass (single);
        }


        std::thread t1 = SaveImageToFileAsync (device, graphicsQueue, commandPool, *outputs.textures[0]->image.image, "test1.png");
        std::thread t2 = SaveImageToFileAsync (device, graphicsQueue, commandPool, *outputs.textures[1]->image.image, "test2.png");

        t1.join ();
        t2.join ();

        std::cout << "OK" << std::endl;
        return EXIT_SUCCESS;
    } catch (std::exception& ex) {
        ERROR (true);
        std::cout << ex.what () << std::endl;
        return EXIT_FAILURE;
    }
}