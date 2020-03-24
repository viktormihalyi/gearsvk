#include "Assert.hpp"
#include "GLFWWindow.hpp"
#include "Logger.hpp"
#include "Noncopyable.hpp"
#include "SDLWindow.hpp"
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
#include "SingleTimeCommand.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"
#include "VulkanUtils.hpp"

#include "ShaderPipeline.hpp"
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


static void debugCallback (
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
{
    std::cout << "validation layer: " << pCallbackData->pMessageIdName << ": " << pCallbackData->pMessage << std::endl
              << std::endl;
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


void CopyBuffer (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    SingleTimeCommand commandBuffer (device, commandPool, graphicsQueue);

    VkBufferCopy copyRegion = {};
    copyRegion.size         = size;
    vkCmdCopyBuffer (commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
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


struct BufferImage {
    Image::U        image;
    DeviceMemory::U memory;
};

DeviceMemory::U AllocateImageMemory (VkPhysicalDevice physicalDevice, VkDevice device, VkImage image, VkMemoryPropertyFlags propertyFlags)
{
    VkMemoryRequirements memRequirements = {};
    vkGetImageMemoryRequirements (device, image, &memRequirements);
    uint32_t memoryTypeIndex = FindMemoryType (physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

    return DeviceMemory::Create (device, memRequirements.size, memoryTypeIndex);
}

BufferImage CreateImage (VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkQueue queue, VkCommandPool commandPool)
{
    BufferMemory stagingMemory = CreateBufferMemory (physicalDevice, device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    {
        MemoryMapping                       bm (device, *stagingMemory.memory, 0, width * height * 4);
        std::vector<std::array<uint8_t, 4>> pixels (width * height);
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                pixels[y * width + x] = {127, 127, 127, 127};
            }
        }

        bm.Copy (pixels);
    }

    BufferImage result;
    result.image  = Image::Create (device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    result.memory = AllocateImageMemory (physicalDevice, device, *result.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (ERROR (vkBindImageMemory (device, *result.image, *result.memory, 0) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to bind buffer memory");
    }

    TransitionImageLayout (device, queue, commandPool, *result.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage (device, queue, commandPool, *stagingMemory.buffer, *result.image, width, height);
    TransitionImageLayout (device, queue, commandPool, *result.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    return std::move (result);
}
/*

int main_OLD (int argc, char* argv[])
{
    std::cout << Utils::GetProjectRoot ().u8string () << std::endl;

    uint32_t apiVersion;
    vkEnumerateInstanceVersion (&apiVersion);

    WindowBase::U window = SDLWindowBase::Create ();

    // platform required extensionss
    std::vector<const char*> extensions;
    {
        extensions = window->GetExtensions ();
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

    Surface        surface (instance, window->CreateSurface (instance));
    PhysicalDevice physicalDevice (instance, surface, Utils::ToSet<const char*, std::string> (requestedDeviceExtensions));
    Device         device (physicalDevice, {*physicalDevice.queueFamilies.graphics, *physicalDevice.queueFamilies.presentation}, requestedDeviceExtensions);
    Swapchain      swapchain (physicalDevice, device, surface);

    Queue graphicsQueue (device, *physicalDevice.queueFamilies.graphics);
    Queue presentQueue (device, *physicalDevice.queueFamilies.presentation);

    CommandPool commandPool (device, *physicalDevice.queueFamilies.graphics);

    const std::vector<Vertex> vertices = {
        Vertex {{-1.f, +1.f}, {1.f, 0.f, 0.f}},
        Vertex {{+1.f, -1.f}, {0.f, 1.f, 0.f}},
        Vertex {{+1.f, +1.f}, {0.f, 0.f, 1.f}},
        Vertex {{-1.f, -1.f}, {1.f, 0.f, 0.f}},
    };

    const size_t bufferSize = vertices.size () * sizeof (Vertex);


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


    ShaderPipeline theShader (device);
    theShader.vertexShader   = ShaderModule::CreateFromSource (device, Utils::GetProjectRoot () / "shaders" / "test.vert");
    theShader.fragmentShader = ShaderModule::CreateFromSource (device, Utils::GetProjectRoot () / "shaders" / "test.frag");


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
        UBOReflection (
            VkPhysicalDevice            physicalDevice,
            VkDevice                    device,
            uint32_t                    imageCount,
            const Gears::UniformData&   uniformData,
            const std::vector<ImgInfo>& imageInfos = {})
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

    BufferImage bimage = CreateImage (physicalDevice, device, 512, 512, graphicsQueue, commandPool);
    ImageView   bimageView (device, *bimage.image);
    Sampler     s (device);

    ImgInfo imgInfo               = {};
    imgInfo.imageInfo.sampler     = s;
    imgInfo.imageInfo.imageView   = bimageView;
    imgInfo.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgInfo.binding               = 2;

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

    RenderPass renderPass (device, {colorAttachment}, {subpass}, {dependency});
    //PipelineLayout pipelineLayout (device, { uniformReflection.GetLayout () });
    PipelineLayout pipelineLayout (device, {});
    Pipeline       pipeline (device, swapchain.extent.width, swapchain.extent.height, 1, pipelineLayout, renderPass, shaderStages, vibds, viads);

    std::vector<Framebuffer::U> swapChainFramebuffers;

    for (size_t i = 0; i < swapchain.imageViews.size (); i++) {
        swapChainFramebuffers.push_back (Framebuffer::Create (
            device,
            renderPass,
            std::vector<std::reference_wrapper<ImageView>> {*swapchain.imageViews[i]},
            swapchain.extent.width,
            swapchain.extent.height));
    }


    std::vector<CommandBuffer::U> commandBuffers;

    for (size_t i = 0; i < swapChainFramebuffers.size (); i++) {
        CommandBuffer::U commandBuffer = CommandBuffer::Create (device, commandPool);

        commandBuffer->Begin ();

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

        //uniformReflection.CmdBindSet (*commandBuffer, pipelineLayout, i);

        vkCmdDraw (*commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass (*commandBuffer);

        commandBuffer->End ();

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


    CopyBuffer (device, graphicsQueue, commandPool, *stagingBuffer.buffer, *vertexBuffer.buffer, bufferSize);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSwapchainKHR       swapChains[] = {swapchain};

    std::chrono::time_point<std::chrono::high_resolution_clock> lastDrawTime = std::chrono::high_resolution_clock::now ();


    size_t currentFrame = 0;
    window->DoEventLoop ([&] () {
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
        //UniformBufferObject ubo    = GetMPV (1.4);
        //uniformReflection ("UniformBufferObject", imageIndex).Copy (ubo);
        //float t = 0.5f;
        //uniformReflection ("Time", imageIndex).Copy (t);

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
        //std::cout << "fps: " << 1.0 / elapsedSecs << std::endl;
        lastDrawTime = currentTime;

        vkQueuePresentKHR (presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    });

    vkDeviceWaitIdle (device);

    return EXIT_SUCCESS;
}


*/
#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"
#include "tests/VulkanTestEnvironment.hpp"

int main (int argc, char* argv[])
{
    WindowBase::U window = SDLWindow::Create ();

    std::vector<const char*> extensions;
    {
        extensions = window->GetExtensions ();
        extensions.push_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    Instance instance (extensions, {"VK_LAYER_KHRONOS_validation"});

    Surface surface (instance, window->CreateSurface (instance));

    TestEnvironment testenv (instance, surface);
    TestCase        testcase (testenv, {VK_KHR_SWAPCHAIN_EXTENSION_NAME});

    Device&      device        = testcase.device;
    CommandPool& commandPool   = testcase.commandPool;
    Queue&       graphicsQueue = testcase.queue;

    Swapchain swapchain (testenv.physicalDevice, device, surface);

    using namespace RenderGraph;
    Graph graph (device, commandPool, GraphSettings (2, window->GetWidth (), window->GetHeight ()));

    auto sp = ShaderPipeline::Create (device);
    sp->AddVertexShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->AddFragmentShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outCopy;

void main () {
    vec4 result = vec4 (1, 0, 0, 1);
    outColor = result;
    outCopy = result;
}
    )");

    Resource& presentedCopy = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings (), device, graphicsQueue, commandPool, std::nullopt, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
    Resource& presented     = graph.CreateResource (SwapchainImageResource::Create (device, swapchain));

    Operation& redFillOperation = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                                  device,
                                                                                  commandPool,
                                                                                  6,
                                                                                  std::move (sp)));

    Operation& presentOp = graph.CreateOperation (PresentOperation::Create (swapchain, graphicsQueue, std::vector<VkSemaphore> {}));

    graph.AddConnection (OutputConnection {redFillOperation, 0, presented});
    graph.AddConnection (OutputConnection {redFillOperation, 1, presentedCopy});

    graph.Compile ();

    Semaphore s (device);

    window->DoEventLoop ([&] () {
        uint32_t imageIndex = 0;
        vkAcquireNextImageKHR (device, swapchain, UINT64_MAX, s, VK_NULL_HANDLE, &imageIndex);
        graph.Submit (graphicsQueue, imageIndex, {s});

        vkQueueWaitIdle (graphicsQueue);
        vkDeviceWaitIdle (device);
    });

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);
}
