#include "Assert.hpp"
#include "GLFWWindowProvider.hpp"
#include "Logger.hpp"
#include "Noncopyable.hpp"
#include "Timer.hpp"
#include "Utils.hpp"

// from VulkanWrapper
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "DebugUtilsMessenger.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "DeviceMemory.hpp"
#include "Fence.hpp"
#include "Framebuffer.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Instance.hpp"
#include "MemoryMapping.hpp"
#include "PhysicalDevice.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "Queue.hpp"
#include "RenderPass.hpp"
#include "Sampler.hpp"
#include "Semaphore.hpp"
#include "ShaderModule.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"

#include "ShaderReflection.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>


#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

constexpr int MAX_FRAMES_IN_FLIGHT = 4;


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessageIdName << ": " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}


struct VertexInfoBase {
    virtual std::vector<VkVertexInputBindingDescription>   GetBindingDescriptions () const   = 0;
    virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions () const = 0;
};


struct Vertex {
    static const std::unique_ptr<VertexInfoBase> info;

    glm::vec2 position;
    glm::vec3 color;
};


struct VertexInfo : public VertexInfoBase {
    std::vector<VkVertexInputBindingDescription> GetBindingDescriptions () const override
    {
        VkVertexInputBindingDescription bindingDescription = {};

        bindingDescription.binding   = 0;
        bindingDescription.stride    = sizeof (Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return {bindingDescription};
    }

    std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions () const override
    {
        std::vector<VkVertexInputAttributeDescription> result;

        {
            VkVertexInputAttributeDescription next;
            next.binding  = 0;
            next.location = 0;
            next.format   = VK_FORMAT_R32G32_SFLOAT;
            next.offset   = offsetof (Vertex, position);

            result.push_back (next);
        }
        {
            VkVertexInputAttributeDescription next;
            next.binding  = 0;
            next.location = 1;
            next.format   = VK_FORMAT_R32G32B32_SFLOAT;
            next.offset   = offsetof (Vertex, color);
            result.push_back (next);
        }

        return result;
    }
};


const std::unique_ptr<VertexInfoBase> Vertex::info = std::make_unique<VertexInfo> ();


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


struct BufferMemory {
    Buffer::U       buffer;
    DeviceMemory::U memory;
    uint32_t        bufferSize;
    uint32_t        allocatedSize;
};


BufferMemory CreateBufferMemory (VkPhysicalDevice physicalDevice, VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags)
{
    BufferMemory result;
    result.bufferSize = bufferSize;
    result.buffer     = Buffer::Create (device, bufferSize, usageFlags);

    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements (device, *result.buffer, &memRequirements);
    uint32_t memoryTypeIndex = FindMemoryType (physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

    result.memory        = DeviceMemory::Create (device, memRequirements.size, memoryTypeIndex);
    result.allocatedSize = memRequirements.size;

    if (ERROR (vkBindBufferMemory (device, *result.buffer, *result.memory, 0) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to bind buffer memory");
    }

    return std::move (result);
}


class SingleTimeCommand final : public Noncopyable {
private:
    const VkDevice      device;
    const VkCommandPool commandPool;
    const VkQueue       queue;
    const CommandBuffer commandBuffer;

public:
    SingleTimeCommand (VkDevice device, VkCommandPool commandPool, VkQueue queue)
        : device (device)
        , queue (queue)
        , commandPool (commandPool)
        , commandBuffer (device, commandPool)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (ERROR (vkBeginCommandBuffer (commandBuffer, &beginInfo) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to begin one time commandbuffer");
        }
    }

    ~SingleTimeCommand ()
    {
        if (ERROR (vkEndCommandBuffer (commandBuffer) != VK_SUCCESS)) {
            return;
        }

        VkCommandBuffer handle = commandBuffer;

        VkSubmitInfo submitInfo       = {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &handle;

        vkQueueSubmit (queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle (queue);
    }

    operator VkCommandBuffer () const { return commandBuffer; }
};


void CopyBuffer (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    SingleTimeCommand commandBuffer (device, commandPool, graphicsQueue);

    VkBufferCopy copyRegion = {};
    copyRegion.size         = size;
    vkCmdCopyBuffer (commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}


void CopyBufferToImage (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    SingleTimeCommand commandBuffer (device, commandPool, graphicsQueue);

    VkBufferImageCopy region = {};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage (
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
}

struct UniformBufferObject {
    glm::mat4 m;
    glm::mat4 v;
    glm::mat4 p;
};


struct UBOTime {
    float time;
};


static UniformBufferObject GetMPV (float asp)
{
    static auto startTime = std::chrono::high_resolution_clock::now ();

    auto  currentTime = std::chrono::high_resolution_clock::now ();
    float time        = std::chrono::duration<float, std::chrono::seconds::period> (currentTime - startTime).count ();

    UniformBufferObject ubo = {};
    ubo.m                   = glm::rotate (glm::mat4 (1.0f), time * glm::radians (90.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
    ubo.v                   = glm::lookAt (glm::vec3 (2.0f, 2.0f, 2.0f), glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
    ubo.p                   = glm::perspective (glm::radians (45.0f), asp, 0.1f, 10.0f);
    ubo.p[1][1] *= -1;
    return ubo;
}


void TransitionImageLayout (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    SingleTimeCommand commandBuffer (device, commandPool, graphicsQueue);
    image.CmdTransitionImageLayout (commandBuffer, oldLayout, newLayout);
}

struct BufferImage {
    Image::U        image;
    DeviceMemory::U memory;
};


BufferImage CreateBufferImage (VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkMemoryPropertyFlags propertyFlags)
{
    BufferImage result;
    result.image = Image::Create (device, width, height);

    VkMemoryRequirements memRequirements = {};
    vkGetImageMemoryRequirements (device, *result.image, &memRequirements);
    uint32_t memoryTypeIndex = FindMemoryType (physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

    result.memory = DeviceMemory::Create (device, memRequirements.size, memoryTypeIndex);

    if (ERROR (vkBindImageMemory (device, *result.image, *result.memory, 0) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to bind buffer memory");
    }

    return std::move (result);
}


namespace RenderGraph {


class Resource : public Noncopyable {
public:
    USING_PTR (Resource);

    uint32_t version;
};

class ImageResource : public Resource {
public:
    USING_PTR (ImageResource);

    VkImage imageHandle;
};

struct Operation : public Noncopyable {
    USING_PTR (Operation);

    std::vector<Resource::Ref> inputs;
    std::vector<Resource::Ref> outputs;

    virtual void Execute () {}
};

struct PresentOperation final : public Operation {
};

struct Render1ToTextureOperation final : public Operation {
    void Execute () override
    {
    }
};

class Graph final : public Noncopyable {
public:
    USING_PTR (Graph);

    std::vector<Resource::U>  resources;
    std::vector<Operation::U> operations;
};
} // namespace RenderGraph


int main (int argc, char* argv[])
{
    {
        using namespace RenderGraph;
        Graph graph;

        Resource::U depthBuffer    = Resource::Create ();
        Resource::U depthBuffer2   = Resource::Create ();
        Resource::U gbuffer1       = Resource::Create ();
        Resource::U gbuffer2       = Resource::Create ();
        Resource::U gbuffer3       = Resource::Create ();
        Resource::U debugOutput    = Resource::Create ();
        Resource::U lightingBuffer = Resource::Create ();
        Resource::U finalTarget    = Resource::Create ();

        Operation::U depthPass   = Operation::Create ();
        Operation::U gbufferPass = Operation::Create ();
        Operation::U debugView   = Operation::Create ();
        Operation::U move        = Operation::Create ();
        Operation::U lighting    = Operation::Create ();
        Operation::U post        = Operation::Create ();
        Operation::U present     = Operation::Create ();

        depthPass->outputs.push_back (*depthBuffer);

        gbufferPass->inputs.push_back (*depthBuffer);
        gbufferPass->outputs.push_back (*depthBuffer2);
        gbufferPass->outputs.push_back (*gbuffer1);
        gbufferPass->outputs.push_back (*gbuffer2);
        gbufferPass->outputs.push_back (*gbuffer3);

        debugView->inputs.push_back (*gbuffer3);
        debugView->outputs.push_back (*debugOutput);

        lighting->inputs.push_back (*depthBuffer);
        lighting->inputs.push_back (*gbuffer1);
        lighting->inputs.push_back (*gbuffer2);
        lighting->inputs.push_back (*gbuffer3);
        lighting->outputs.push_back (*lightingBuffer);

        post->inputs.push_back (*lightingBuffer);
        post->outputs.push_back (*finalTarget);

        move->inputs.push_back (*debugOutput);
        move->outputs.push_back (*finalTarget);

        present->inputs.push_back (*finalTarget);
    }

    std::cout << Utils::PROJECT_ROOT.u8string () << std::endl;

    uint32_t apiVersion;
    vkEnumerateInstanceVersion (&apiVersion);

    WindowProvider::U windowProvider = GLFWWindowProvider::Create ();

    // platform required extensionss
    std::vector<const char*> extensions;
    {
        extensions = windowProvider->GetExtensions ();
        extensions.push_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    const std::vector<const char*> requestedDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    Instance instance (extensions, validationLayers);

    DebugUtilsMessenger debugMessenger (instance, debugCallback);

    Surface        surface (instance, windowProvider->CreateSurface (instance));
    PhysicalDevice physicalDevice (instance, surface, Utils::ToSet<const char*, std::string> (requestedDeviceExtensions));
    Device         device (physicalDevice, *physicalDevice.queueFamilies.graphics, requestedDeviceExtensions);
    Swapchain      swapchain (physicalDevice, device, surface);

    const std::vector<Vertex> vertices = {
        Vertex {{-1.f, +1.f}, {1.f, 0.f, 0.f}},
        Vertex {{+1.f, -1.f}, {0.f, 1.f, 0.f}},
        Vertex {{+1.f, +1.f}, {0.f, 0.f, 1.f}},
        Vertex {{-1.f, -1.f}, {1.f, 0.f, 0.f}},
    };

    const size_t bufferSize = vertices.size () * sizeof (Vertex);
    /*
    BufferImage bimage = CreateBufferImage (physicalDevice, device, 512, 512, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    {
        MemoryMapping                       bm (device, *bimage.memory, 0, 512 * 512 * 4);
        std::vector<std::array<uint8_t, 4>> pixels (512 * 512);
        for (uint32_t y = 0; y < 512; ++y) {
            for (uint32_t x = 0; x < 512; ++x) {
                pixels[y * 512 + x] = {0, 0, 0, 0};
            }
        }

        bm.Copy (pixels);
    }
    ImageView bimageView (device, *bimage.image);
    */


    BufferMemory stagingBuffer = CreateBufferMemory (
        physicalDevice, device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    BufferMemory vertexBuffer = CreateBufferMemory (
        physicalDevice, device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    MemoryMapping mapping (device, *stagingBuffer.memory, 0, bufferSize);
    mapping.Copy (vertices);


    struct ShaderPipeline {
        ShaderModule::U vertexShader;
        ShaderModule::U fragmentShader;
        ShaderModule::U geometryShader;
        ShaderModule::U tessellationEvaluationShader;
        ShaderModule::U tessellationControlShader;
        ShaderModule::U computeShader;

        std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages () const
        {
            std::vector<VkPipelineShaderStageCreateInfo> result;

            if (vertexShader != nullptr)
                result.push_back (vertexShader->GetShaderStageCreateInfo ());
            if (fragmentShader != nullptr)
                result.push_back (fragmentShader->GetShaderStageCreateInfo ());
            if (geometryShader != nullptr)
                result.push_back (geometryShader->GetShaderStageCreateInfo ());
            if (tessellationEvaluationShader != nullptr)
                result.push_back (tessellationEvaluationShader->GetShaderStageCreateInfo ());
            if (tessellationControlShader != nullptr)
                result.push_back (tessellationControlShader->GetShaderStageCreateInfo ());
            if (computeShader != nullptr)
                result.push_back (computeShader->GetShaderStageCreateInfo ());

            return result;
        }
    };


    ShaderPipeline theShader;
    theShader.vertexShader   = ShaderModule::CreateFromSource (device, Utils::PROJECT_ROOT / "shaders" / "shader.vert");
    theShader.fragmentShader = ShaderModule::CreateFromSource (device, Utils::PROJECT_ROOT / "shaders" / "shader.frag");


    struct ImgInfo {
        VkDescriptorImageInfo imageInfo;
        uint32_t              binding;
    };

    class UBOReflection {
    private:
        struct UniformDataForSwapchain {
            // one for each swapchain image
            std::vector<BufferMemory>     buffers;
            std::vector<MemoryMapping::U> mappings;

            Gears::UniformData::Block reflection;
        };

        DescriptorPool::U      descriptorPool;
        DescriptorSetLayout::U layout;

        std::unordered_map<std::string, UniformDataForSwapchain> uniforms;

        // one for each swapchain
        std::vector<DescriptorSet::U> descriptorSets;

    public:
        UBOReflection (VkPhysicalDevice physicalDevice, VkDevice device, uint32_t imageCount, const Gears::UniformData& uniformData, const std::vector<ImgInfo>& imageInfos = {})
        {
            const uint32_t maxSets = imageCount;
            descriptorPool         = DescriptorPool::Create (device,
                                                     imageCount * (uniformData.ubos.size ()),
                                                     imageCount * (uniformData.samplers.size ()),
                                                     maxSets);


            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

            for (const Gears::UniformData::Sampler& s : uniformData.samplers) {
                layoutBindings.push_back (s.GetDescriptorSetLayoutBinding ());
            }

            for (const Gears::UniformData::Block& ubo : uniformData.ubos) {
                uniforms[ubo.name].reflection = ubo;

                layoutBindings.push_back (ubo.GetDescriptorSetLayoutBinding ());

                for (int i = 0; i < imageCount; ++i) {
                    BufferMemory b = CreateBufferMemory (
                        physicalDevice, device,
                        ubo.blockSize,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                    uniforms[ubo.name].mappings.push_back (MemoryMapping::Create (device, *b.memory, 0, b.bufferSize));
                    uniforms[ubo.name].buffers.push_back (std::move (b));
                }
            }

            layout = DescriptorSetLayout::Create (device, layoutBindings);

            for (int i = 0; i < imageCount; ++i) {
                descriptorSets.push_back (DescriptorSet::Create (device, *descriptorPool, *layout));
            }

            for (const auto& [uboName, uboData] : uniforms) {
                for (size_t i = 0; i < imageCount; ++i) {
                    VkDescriptorBufferInfo bufferInfo;
                    bufferInfo.buffer = *uboData.buffers[i].buffer;
                    bufferInfo.offset = 0;
                    bufferInfo.range  = uboData.buffers[i].bufferSize;

                    descriptorSets[i]->WriteOneBufferInfo (uniforms[uboName].reflection.binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfo);
                }
            }

            for (const auto& imageInfo : imageInfos) {
                for (const auto& set : descriptorSets) {
                    set->WriteOneImageInfo (imageInfo.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageInfo.imageInfo);
                }
            }
        }

        UniformDataForSwapchain& operator[] (const std::string& name)
        {
            return uniforms[name];
        }

        MemoryMapping& operator() (const std::string& name, uint32_t imageIndex)
        {
            return *uniforms[name].mappings[imageIndex];
        }

        void CmdBindSet (const CommandBuffer& commandBuffer, const PipelineLayout& pipelineLayout, uint32_t index) const
        {
            VkDescriptorSet ds = *descriptorSets[index];
            vkCmdBindDescriptorSets (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                                     1, &ds,
                                     0, nullptr);
        }

        const DescriptorSetLayout& GetLayout () const
        {
            return *layout;
        }
    };

    Gears::ShaderReflection refl (theShader.vertexShader->GetBinary ());
    Gears::ShaderReflection reflf (theShader.fragmentShader->GetBinary ());
    //Gears::ShaderReflection reflfU ({theShader.vertexShader->GetBinary (), theShader.fragmentShader->GetBinary ()});

    //Sampler s (device);
    //
    //ImgInfo imgInfo               = {};
    //imgInfo.imageInfo.sampler     = s;
    //imgInfo.imageInfo.imageView   = bimageView;
    //imgInfo.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //imgInfo.binding               = 4;

    //UBOReflection uniformReflection (physicalDevice, device, swapchain.GetImageCount (), refl.Get (), {imgInfo});
    //UBOReflection uniformReflectionFr (physicalDevice, device, swapchain.GetImageCount (), reflf.Get (), {});


    std::vector<VkPipelineShaderStageCreateInfo>   shaderStages = theShader.GetShaderStages ();
    std::vector<VkVertexInputBindingDescription>   vibds        = Vertex::info->GetBindingDescriptions ();
    std::vector<VkVertexInputAttributeDescription> viads        = Vertex::info->GetAttributeDescriptions ();


    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format                  = swapchain.surfaceFormat.format;
    colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment            = 0;
    colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask       = 0;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount        = 1;
    renderPassInfo.pAttachments           = &colorAttachment;
    renderPassInfo.subpassCount           = 1;
    renderPassInfo.pSubpasses             = &subpass;
    renderPassInfo.dependencyCount        = 1;
    renderPassInfo.pDependencies          = &dependency;

    RenderPass     renderPass (device, {colorAttachment}, {subpass}, {dependency});
    PipelineLayout pipelineLayout (device, {/*uniformReflectionFr.GetLayout ()*/});
    Pipeline       pipeline (device, swapchain.extent.width, swapchain.extent.height, pipelineLayout, renderPass, shaderStages, vibds, viads);

    std::vector<Framebuffer::U> swapChainFramebuffers;

    for (size_t i = 0; i < swapchain.imageViews.size (); i++) {
        swapChainFramebuffers.push_back (Framebuffer::Create (
            device,
            renderPass,
            std::vector<std::reference_wrapper<ImageView>> {*swapchain.imageViews[i]},
            swapchain.extent.width,
            swapchain.extent.height));
    }


    CommandPool commandPool (device, *physicalDevice.queueFamilies.graphics);

    std::vector<CommandBuffer::U> commandBuffers;

    for (size_t i = 0; i < swapChainFramebuffers.size (); i++) {
        CommandBuffer::U commandBuffer = CommandBuffer::Create (device, commandPool);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = 0;       // Optional
        beginInfo.pInheritanceInfo         = nullptr; // Optional

        if (ERROR (vkBeginCommandBuffer (*commandBuffer, &beginInfo) != VK_SUCCESS)) {
            return EXIT_FAILURE;
        }

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass            = renderPass;
        renderPassInfo.framebuffer           = *swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset     = {0, 0};
        renderPassInfo.renderArea.extent     = swapchain.extent;
        renderPassInfo.clearValueCount       = 1;
        renderPassInfo.pClearValues          = &clearColor;

        vkCmdBeginRenderPass (*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline (*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkBuffer     vertexBuffers[] = {*vertexBuffer.buffer};
        VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers (*commandBuffer, 0, 1, vertexBuffers, offsets);

        //uniformReflectionFr.CmdBindSet (*commandBuffer, pipelineLayout, i);

        vkCmdDraw (*commandBuffer, 4, 1, 0, 0);

        vkCmdEndRenderPass (*commandBuffer);

        if (ERROR (vkEndCommandBuffer (*commandBuffer) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to end commandbuffer");
        }

        commandBuffers.push_back (std::move (commandBuffer));
    }

    const std::vector<VkCommandBuffer> cmdBufferHandles = Utils::ConvertToHandles<CommandBuffer, VkCommandBuffer> (commandBuffers);


    std::vector<Semaphore::U> imageAvailableSemaphore;
    std::vector<Semaphore::U> renderFinishedSemaphore;
    std::vector<Fence::U>     inFlightFences;
    std::vector<VkFence>      imagesInFlight (swapchain.imageViews.size ());

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        imageAvailableSemaphore.push_back (Semaphore::Create (device));
        renderFinishedSemaphore.push_back (Semaphore::Create (device));
        inFlightFences.push_back (Fence::Create (device));
    }

    const std::vector<VkFence> inFlightFenceHandles = Utils::ConvertToHandles<Fence, VkFence> (inFlightFences);


    Queue graphicsQueue (device, *physicalDevice.queueFamilies.graphics);
    Queue presentQueue (device, *physicalDevice.queueFamilies.presentation);

    //TransitionImageLayout (device, graphicsQueue, commandPool, *bimage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //TransitionImageLayout (device, graphicsQueue, commandPool, *bimage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //
    CopyBuffer (device, graphicsQueue, commandPool, *stagingBuffer.buffer, *vertexBuffer.buffer, bufferSize);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSwapchainKHR       swapChains[] = {swapchain};

    std::chrono::time_point<std::chrono::high_resolution_clock> lastDrawTime = std::chrono::high_resolution_clock::now ();

    size_t currentFrame = 0;
    windowProvider->DoEventLoop ([&] () {
        vkWaitForFences (device, 1, &inFlightFenceHandles[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences (device, 1, &inFlightFenceHandles[currentFrame]);

        uint32_t imageIndex = 0;
        vkAcquireNextImageKHR (device, swapchain, UINT64_MAX, *imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences (device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        imagesInFlight[imageIndex] = *inFlightFences[currentFrame];
        /*
        uniformReflectionFr ("UniformBufferObject", imageIndex).Copy (ubo);
        uniformReflection ("Time", imageIndex).Copy (t);
        */

        VkSemaphore waitSemaphores[]   = {*imageAvailableSemaphore[currentFrame]};
        VkSemaphore signalSemaphores[] = {*renderFinishedSemaphore[currentFrame]};

        VkSubmitInfo submitInfo         = {};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &cmdBufferHandles[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        vkResetFences (device, 1, &inFlightFenceHandles[currentFrame]);

        if (ERROR (vkQueueSubmit (graphicsQueue, 1, &submitInfo, *inFlightFences[currentFrame]) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to submit queue");
        }

        VkPresentInfoKHR presentInfo   = {};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = swapChains;
        presentInfo.pImageIndices      = &imageIndex;
        presentInfo.pResults           = nullptr; // Optional

        auto   currentTime     = std::chrono::high_resolution_clock::now ();
        auto   elapsedNanosecs = (std::chrono::duration_cast<std::chrono::nanoseconds> (currentTime - lastDrawTime)).count ();
        double elapsedSecs     = elapsedNanosecs * 1e-9;
        std::cout << "fps: " << 1.0 / elapsedSecs << std::endl;
        lastDrawTime = currentTime;

        vkQueuePresentKHR (presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    });

    vkDeviceWaitIdle (device);

    return EXIT_SUCCESS;
}
