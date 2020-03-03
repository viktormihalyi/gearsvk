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
#include "ImageView.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "RenderPass.hpp"
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
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}


struct PipelineCreateResultU {
    std::unique_ptr<Pipeline>       handle;
    std::unique_ptr<RenderPass>     renderPass;
    std::unique_ptr<PipelineLayout> pipelineLayout;
};


struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription GetBindingDescription ()
    {
        VkVertexInputBindingDescription bindingDescription = {};

        bindingDescription.binding   = 0;
        bindingDescription.stride    = sizeof (Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions ()
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


struct BufferMemoryU {
    std::unique_ptr<Buffer>       buffer;
    std::unique_ptr<DeviceMemory> memory;
};


BufferMemoryU CreateBufferMemoryU (VkPhysicalDevice physicalDevice, VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags)
{
    BufferMemoryU result;
    result.buffer = std::make_unique<Buffer> (device, bufferSize, usageFlags);

    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements (device, *result.buffer, &memRequirements);
    uint32_t memoryTypeIndex = FindMemoryType (physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

    result.memory = std::make_unique<DeviceMemory> (device, memRequirements.size, memoryTypeIndex);

    if (ERROR (vkBindBufferMemory (device, *result.buffer, *result.memory, 0) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to bind buffer memory");
    }

    return std::move (result);
}


void CopyBuffer (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool                 = commandPool;
    allocInfo.commandBufferCount          = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers (device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer (commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size         = size;
    vkCmdCopyBuffer (commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer (commandBuffer);

    VkSubmitInfo submitInfo       = {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit (graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle (graphicsQueue);

    vkFreeCommandBuffers (device, commandPool, 1, &commandBuffer);
}


class MemoryMapping : public Noncopyable {
private:
    const VkDevice       device;
    const VkDeviceMemory memory;

    void* mappedMemory;

public:
    MemoryMapping (VkDevice device, VkDeviceMemory memory, uint32_t offset, uint32_t size)
        : device (device)
        , memory (memory)
        , mappedMemory (nullptr)

    {
        if (ERROR (vkMapMemory (device, memory, offset, size, 0, &mappedMemory) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to map memory");
        }
    }

    ~MemoryMapping ()
    {
        vkUnmapMemory (device, memory);
    }

    void* Get () const
    {
        return mappedMemory;
    }
};


void UploadToHostCoherent (VkDevice device, VkDeviceMemory memory, uint32_t offset, const void* data, size_t bytes)
{
    MemoryMapping mapping (device, memory, offset, bytes);
    memcpy (mapping.Get (), data, bytes);
}


struct UniformBufferObject {
    glm::mat4 m;
    glm::mat4 v;
    glm::mat4 p;
};


struct UBOTime {
    float time;
};


void UpdateUniformBuffer (VkDevice device, VkDeviceMemory memory, float asp, uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now ();

    auto  currentTime = std::chrono::high_resolution_clock::now ();
    float time        = std::chrono::duration<float, std::chrono::seconds::period> (currentTime - startTime).count ();

    Utils::DummyTimerLogger logger;
    {
        Utils::TimerScope timerScope (logger);

        UniformBufferObject ubo = {};
        ubo.m                   = glm::rotate (glm::mat4 (1.0f), time * glm::radians (90.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
        ubo.v                   = glm::lookAt (glm::vec3 (2.0f, 2.0f, 2.0f), glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
        ubo.p                   = glm::perspective (glm::radians (45.0f), asp, 0.1f, 10.0f);
        ubo.p[1][1] *= -1;
        UploadToHostCoherent (device, memory, 0, &ubo, sizeof (ubo));
    }
}


template<typename T>
std::vector<std::reference_wrapper<T>> To (const std::vector<std::unique_ptr<T>>& src)
{
    std::vector<std::reference_wrapper<T>> result;
    for (const auto& a : src) {
        result.push_back (a);
    }
    return result;
}


int main (int argc, char* argv[])
{
    std::cout << Utils::PROJECT_ROOT.u8string () << std::endl;

    uint32_t apiVersion;
    vkEnumerateInstanceVersion (&apiVersion);

    std::unique_ptr<WindowProvider> windowProvider = std::make_unique<GLFWWindowProvider> ();

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

    BufferMemoryU stagingBuffer = CreateBufferMemoryU (
        physicalDevice, device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    BufferMemoryU vertexBuffer = CreateBufferMemoryU (
        physicalDevice, device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    UploadToHostCoherent (device, *stagingBuffer.memory, 0, vertices.data (), bufferSize);


    struct Shader {
        std::unique_ptr<ShaderModule> vertexShader;
        std::unique_ptr<ShaderModule> fragmentShader;
        std::unique_ptr<ShaderModule> geometryShader;
        std::unique_ptr<ShaderModule> tessellationEvaluationShader;
        std::unique_ptr<ShaderModule> tessellationControlShader;
        std::unique_ptr<ShaderModule> computeShader;

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

    struct VertexBase {
        virtual std::vector<VkVertexInputBindingDescription> GetBindingDescriptions () const   = 0;
        virtual std::vector<VkVertexInputBindingDescription> GetAttributeDescriptions () const = 0;
    };

    Shader theShader;
    theShader.vertexShader   = std::make_unique<ShaderModule> (device, Utils::PROJECT_ROOT / "shaders" / "shader.vert");
    theShader.fragmentShader = std::make_unique<ShaderModule> (device, Utils::PROJECT_ROOT / "shaders" / "shader.frag");

    Gears::ShaderReflection refl (theShader.vertexShader->binary);

    std::vector<VkDescriptorSetLayoutBinding> uboLayoutBindings;
    for (const auto& ubo : refl.GeUniformData ()) {
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding                      = ubo.binding;
        uboLayoutBinding.descriptorCount              = 1;
        uboLayoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers           = nullptr;
        uboLayoutBinding.stageFlags                   = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBindings.push_back (uboLayoutBinding);
    }

    DescriptorSetLayout layout_wt (device, uboLayoutBindings);


    std::vector<VkPipelineShaderStageCreateInfo>   shaderStages = theShader.GetShaderStages ();
    std::vector<VkVertexInputBindingDescription>   vibds        = {Vertex::GetBindingDescription ()};
    std::vector<VkVertexInputAttributeDescription> viads        = Vertex::GetAttributeDescriptions ();


    VkDescriptorSetLayoutBinding uboLayoutBinding1 = {};
    uboLayoutBinding1.binding                      = 0;
    uboLayoutBinding1.descriptorCount              = 1;
    uboLayoutBinding1.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding1.pImmutableSamplers           = nullptr;
    uboLayoutBinding1.stageFlags                   = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding uboLayoutBinding2 = {};
    uboLayoutBinding2.binding                      = 1;
    uboLayoutBinding2.descriptorCount              = 1;
    uboLayoutBinding2.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding2.pImmutableSamplers           = nullptr;
    uboLayoutBinding2.stageFlags                   = VK_SHADER_STAGE_VERTEX_BIT;

    DescriptorSetLayout layout (device, {uboLayoutBinding1, uboLayoutBinding2});

    std::vector<BufferMemoryU> uniformBuffers;
    std::vector<BufferMemoryU> uniformBuffersTime;


    for (int i = 0; i < swapchain.GetImageCount (); ++i) {
        uniformBuffers.push_back (CreateBufferMemoryU (
            physicalDevice, device,
            sizeof (UniformBufferObject),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        uniformBuffersTime.push_back (CreateBufferMemoryU (
            physicalDevice, device,
            sizeof (UBOTime),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    }

    DescriptorPool descriptorPool (device, swapchain.GetImageCount (), swapchain.GetImageCount ());

    std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
    for (int i = 0; i < swapchain.GetImageCount (); ++i) {
        descriptorSets.push_back (std::make_unique<DescriptorSet> (device, descriptorPool, layout));
    }

    const std::vector<VkDescriptorSet> descriptorSetsHandles = Utils::ConvertToHandles<DescriptorSet, VkDescriptorSet> (descriptorSets);

    for (size_t i = 0; i < swapchain.GetImageCount (); ++i) {
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = *uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof (UniformBufferObject);

        VkDescriptorBufferInfo bufferInfo2;
        bufferInfo2.buffer = *uniformBuffersTime[i].buffer;
        bufferInfo2.offset = 0;
        bufferInfo2.range  = sizeof (UBOTime);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet               = *descriptorSets[i];
        descriptorWrite.dstBinding           = 0;
        descriptorWrite.dstArrayElement      = 0;
        descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount      = 1;
        descriptorWrite.pBufferInfo          = &bufferInfo;
        descriptorWrite.pImageInfo           = nullptr; // Optional
        descriptorWrite.pTexelBufferView     = nullptr; // Optional

        VkWriteDescriptorSet descriptorWrite2 = {};
        descriptorWrite2.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite2.dstSet               = *descriptorSets[i];
        descriptorWrite2.dstBinding           = 1;
        descriptorWrite2.dstArrayElement      = 0;
        descriptorWrite2.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite2.descriptorCount      = 1;
        descriptorWrite2.pBufferInfo          = &bufferInfo2;
        descriptorWrite2.pImageInfo           = nullptr; // Optional
        descriptorWrite2.pTexelBufferView     = nullptr; // Optional

        vkUpdateDescriptorSets (device, 1, &descriptorWrite, 0, nullptr);
        vkUpdateDescriptorSets (device, 1, &descriptorWrite2, 0, nullptr);
    }

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
    PipelineLayout pipelineLayout (device, {layout});
    Pipeline       pipeline (device, swapchain.extent.width, swapchain.extent.height, pipelineLayout, renderPass, shaderStages, vibds, viads);

    std::vector<std::unique_ptr<Framebuffer>> swapChainFramebuffers;

    for (size_t i = 0; i < swapchain.imageViews.size (); i++) {
        swapChainFramebuffers.push_back (std::make_unique<Framebuffer> (
            device,
            renderPass,
            std::vector<std::reference_wrapper<ImageView>> {*swapchain.imageViews[i]},
            swapchain.extent.width,
            swapchain.extent.height));
    }


    CommandPool commandPool (device, *physicalDevice.queueFamilies.graphics);

    std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;

    for (size_t i = 0; i < swapChainFramebuffers.size (); i++) {
        std::unique_ptr<CommandBuffer> commandBuffer = std::make_unique<CommandBuffer> (device, commandPool);

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

        vkCmdBindDescriptorSets (*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetsHandles[i], 0, nullptr);

        vkCmdDraw (*commandBuffer, 4, 1, 0, 0);

        vkCmdEndRenderPass (*commandBuffer);

        if (ERROR (vkEndCommandBuffer (*commandBuffer) != VK_SUCCESS)) {
            return EXIT_FAILURE;
        }

        commandBuffers.push_back (std::move (commandBuffer));
    }

    const std::vector<VkCommandBuffer> cmdBufferHandles = Utils::ConvertToHandles<CommandBuffer, VkCommandBuffer> (commandBuffers);


    std::vector<std::unique_ptr<Semaphore>> imageAvailableSemaphore;
    std::vector<std::unique_ptr<Semaphore>> renderFinishedSemaphore;
    std::vector<std::unique_ptr<Fence>>     inFlightFences;
    std::vector<VkFence>                    imagesInFlight (swapchain.imageViews.size ());

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        imageAvailableSemaphore.push_back (std::make_unique<Semaphore> (device));
        renderFinishedSemaphore.push_back (std::make_unique<Semaphore> (device));
        inFlightFences.push_back (std::make_unique<Fence> (device));
    }

    const std::vector<VkFence> inFlightFenceHandles = Utils::ConvertToHandles<Fence, VkFence> (inFlightFences);


    class Queue : public Noncopyable {
    private:
        VkQueue handle;

    public:
        Queue (VkDevice device, uint32_t index)
        {
            vkGetDeviceQueue (device, index, 0, &handle); // TODO another index
        }

        operator VkQueue () const
        {
            return handle;
        }
    };


    Queue graphicsQueue (device, *physicalDevice.queueFamilies.graphics);
    Queue presentQueue (device, *physicalDevice.queueFamilies.presentation);

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

        UpdateUniformBuffer (device, *uniformBuffers[imageIndex].memory, static_cast<float> (swapchain.extent.width) / swapchain.extent.height, imageIndex);
        float t = 1.0f;
        UploadToHostCoherent (device, *uniformBuffersTime[imageIndex].memory, 0, &t, sizeof (t));

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