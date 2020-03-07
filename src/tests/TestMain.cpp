#include <vulkan/vulkan.h>

#include "ShaderPipeline.hpp"
#include "VulkanWrapper.hpp"

#include <iostream>
#include <optional>
#include <string>

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
    std::cerr << "validation layer: " << pCallbackData->pMessageIdName << ": " << pCallbackData->pMessage << std::endl
              << std::endl;
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

int main ()
{
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

    // platform required extensionss
    const std::vector<const char*> extensions       = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    Instance            instance (extensions, validationLayers);
    DebugUtilsMessenger messenger (instance, debugCallback);
    PhysicalDevice      physicalDevice (instance, {}, {acceptGraphicsBit, acceptAnything, acceptAnything, acceptAnything});
    Device              device (physicalDevice, *physicalDevice.queueFamilies.graphics, {});
    Queue               graphicsQueue (device, *physicalDevice.queueFamilies.graphics);
    CommandPool         commandPool (device, *physicalDevice.queueFamilies.graphics);


    Image        presentedImage (device, 512, 512, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    DeviceMemory gpuMemory = AllocateImageMemory (physicalDevice, device, presentedImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindImageMemory (device, presentedImage, gpuMemory, 0);

    Image        dstImage (device, 512, 512, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    DeviceMemory dstMemory = AllocateImageMemory (physicalDevice, device, dstImage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindImageMemory (device, dstImage, dstMemory, 0);
    TransitionImageLayout (device, graphicsQueue, commandPool, dstImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    Buffer       cpuBuffer (device, 512 * 512 * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    DeviceMemory cpuMemory = AllocateBufferMemory (physicalDevice, device, cpuBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory (device, cpuBuffer, cpuMemory, 0);

    ImageView presentedImageView (device, presentedImage);


    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format                  = VK_FORMAT_R8G8B8A8_SRGB;
    colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment            = 0;
    colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = 0;
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

    ShaderPipeline shaders;
    shaders.vertexShader   = ShaderModule::CreateFromSource (device, Utils::GetProjectRoot () / "shaders" / "test.vert");
    shaders.fragmentShader = ShaderModule::CreateFromSource (device, Utils::GetProjectRoot () / "shaders" / "test.frag");

    RenderPass     renderPass (device, {colorAttachment}, {subpass}, {dependency});
    PipelineLayout pipelineLayout (device, {});
    Pipeline       pipeline (device, 512, 512, pipelineLayout, renderPass, shaders.GetShaderStages (), {}, {});

    CommandBuffer commandBuffer (device, commandPool);
    commandBuffer.Begin ();

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

    Framebuffer framebuffer (device, renderPass, {presentedImageView.operator VkImageView ()}, 512, 512);

    {
        SingleTimeCommand single (device, commandPool, graphicsQueue);

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass            = renderPass;
        renderPassBeginInfo.framebuffer           = framebuffer;
        renderPassBeginInfo.renderArea.offset     = {0, 0};
        renderPassBeginInfo.renderArea.extent     = {512, 512};
        renderPassBeginInfo.clearValueCount       = 1;
        renderPassBeginInfo.pClearValues          = &clearColor;

        vkCmdBeginRenderPass (single, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline (single, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw (single, 3, 1, 0, 0);
        vkCmdEndRenderPass (single);
    }

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    {
        SingleTimeCommand single (device, commandPool, graphicsQueue);
        VkBufferImageCopy copy               = {};
        copy.bufferOffset                    = 0;
        copy.bufferRowLength                 = 0;
        copy.bufferImageHeight               = 0;
        copy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.imageSubresource.mipLevel       = 0;
        copy.imageSubresource.baseArrayLayer = 0;
        copy.imageSubresource.layerCount     = 1;
        copy.imageOffset                     = {0, 0, 0};
        copy.imageExtent                     = {512, 512, 1};
        vkCmdCopyImageToBuffer (single, presentedImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cpuBuffer, 1, &copy);
    }

    {
        SingleTimeCommand single (device, commandPool, graphicsQueue);

        VkImageCopy imageCopyRegion               = {};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width              = 512;
        imageCopyRegion.extent.height             = 512;
        imageCopyRegion.extent.depth              = 1;

        vkCmdCopyImage (
            single,
            presentedImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    {
        MemoryMapping        mapping (device, cpuMemory, 0, 512 * 512 * 4);
        std::vector<uint8_t> mapped (512 * 512 * 4);
        memcpy (mapped.data (), mapping.Get (), 512 * 512 * 4);
        stbi_write_jpg ("testBuf.jpg", 512, 512, 4, mapping.Get (), 90);
    }

    {
        MemoryMapping        mapping (device, dstMemory, 0, 512 * 512 * 4);
        std::vector<uint8_t> mapped (512 * 512 * 4);
        memcpy (mapped.data (), mapping.Get (), 512 * 512 * 4);
        stbi_write_jpg ("testImg.jpg", 512, 512, 4, mapping.Get (), 90);
    }


    std::cout << "OK" << std::endl;
    return EXIT_SUCCESS;
}