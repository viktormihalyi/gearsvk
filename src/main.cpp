#include "Assert.hpp"
#include "GLFWWindowProvider.hpp"
#include "Logger.hpp"
#include "Noncopyable.hpp"
#include "ShaderModule.hpp"
#include "Timer.hpp"
#include "Utils.hpp"

#include "Device.hpp"
#include "Framebuffer.hpp"
#include "ImageView.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
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

#include <nlohmann/json.hpp>

#include <glm/glm.hpp>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

using json = nlohmann::json;


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

VkResult CreateDebugUtilsMessengerEXT (VkInstance                                instance,
                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                       const VkAllocationCallbacks*              pAllocator,
                                       VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func (instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void DestroyDebugUtilsMessengerEXT (VkInstance                   instance,
                                    VkDebugUtilsMessengerEXT     debugMessenger,
                                    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func (instance, debugMessenger, pAllocator);
    }
}


struct PipelineCreateResult {
    VkPipeline       handle;
    VkRenderPass     renderPass;
    VkPipelineLayout pipelineLayout;
};


PipelineCreateResult CreateGraphicsPipeline (
    VkDevice                                              device,
    Swapchain::SwapchainCreateResult                      swapchain,
    const std::vector<VkPipelineShaderStageCreateInfo>&   shaderStages,
    const std::vector<VkVertexInputBindingDescription>&   vertexBindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions,
    const std::vector<VkDescriptorSetLayout>&             descriptorSetLayouts)
{
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH,
    };


    VkPipelineVertexInputStateCreateInfo vertexInputInfo     = {};
    vertexInputInfo.sType                                    = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount            = static_cast<uint32_t> (vertexBindingDescriptions.size ());
    vertexInputInfo.pVertexBindingDescriptions               = vertexBindingDescriptions.data ();
    vertexInputInfo.vertexAttributeDescriptionCount          = static_cast<uint32_t> (vertexAttributeDescriptions.size ());
    vertexInputInfo.pVertexAttributeDescriptions             = vertexAttributeDescriptions.data ();
    VkPipelineInputAssemblyStateCreateInfo inputAssembly     = {};
    inputAssembly.sType                                      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology                                   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable                     = VK_FALSE;
    VkViewport viewport                                      = {};
    viewport.x                                               = 0.0f;
    viewport.y                                               = 0.0f;
    viewport.width                                           = static_cast<float> (swapchain.extent.width);
    viewport.height                                          = static_cast<float> (swapchain.extent.height);
    viewport.minDepth                                        = 0.0f;
    viewport.maxDepth                                        = 1.0f;
    VkRect2D scissor                                         = {};
    scissor.offset                                           = {0, 0};
    scissor.extent                                           = swapchain.extent;
    VkPipelineViewportStateCreateInfo viewportState          = {};
    viewportState.sType                                      = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount                              = 1;
    viewportState.pViewports                                 = &viewport;
    viewportState.scissorCount                               = 1;
    viewportState.pScissors                                  = &scissor;
    VkPipelineRasterizationStateCreateInfo rasterizer        = {};
    rasterizer.sType                                         = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable                              = VK_FALSE;
    rasterizer.rasterizerDiscardEnable                       = VK_FALSE;
    rasterizer.polygonMode                                   = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                                     = 1.0f;
    rasterizer.cullMode                                      = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace                                     = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable                               = VK_FALSE;
    rasterizer.depthBiasConstantFactor                       = 0.0f; // Optional
    rasterizer.depthBiasClamp                                = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor                          = 0.0f; // Optional
    VkPipelineMultisampleStateCreateInfo multisampling       = {};
    multisampling.sType                                      = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable                        = VK_FALSE;
    multisampling.rasterizationSamples                       = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading                           = 1.0f;     // Optional
    multisampling.pSampleMask                                = nullptr;  // Optional
    multisampling.alphaToCoverageEnable                      = VK_FALSE; // Optional
    multisampling.alphaToOneEnable                           = VK_FALSE; // Optional
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask                      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable                         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor                 = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor                 = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp                        = VK_BLEND_OP_ADD;      // Optional
    colorBlendAttachment.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp                        = VK_BLEND_OP_ADD;      // Optional
    colorBlendAttachment.blendEnable                         = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor                 = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor                 = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp                        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp                        = VK_BLEND_OP_ADD;
    VkPipelineColorBlendStateCreateInfo colorBlending        = {};
    colorBlending.sType                                      = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable                              = VK_FALSE;
    colorBlending.logicOp                                    = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount                            = 1;
    colorBlending.pAttachments                               = &colorBlendAttachment;
    colorBlending.blendConstants[0]                          = 0.0f; // Optional
    colorBlending.blendConstants[1]                          = 0.0f; // Optional
    colorBlending.blendConstants[2]                          = 0.0f; // Optional
    colorBlending.blendConstants[3]                          = 0.0f; // Optional
    VkPipelineDynamicStateCreateInfo dynamicState            = {};
    dynamicState.sType                                       = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount                           = static_cast<uint32_t> (dynamicStates.size ());
    dynamicState.pDynamicStates                              = dynamicStates.data ();
    VkPipelineLayoutCreateInfo pipelineLayoutInfo            = {};
    pipelineLayoutInfo.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount                        = static_cast<uint32_t> (descriptorSetLayouts.size ());
    pipelineLayoutInfo.pSetLayouts                           = descriptorSetLayouts.data ();
    pipelineLayoutInfo.pushConstantRangeCount                = 0;       // Optional
    pipelineLayoutInfo.pPushConstantRanges                   = nullptr; // Optional

    PipelineCreateResult result;

    if (ERROR (vkCreatePipelineLayout (device, &pipelineLayoutInfo, nullptr, &result.pipelineLayout) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create pipeline layout");
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

    if (ERROR (vkCreateRenderPass (device, &renderPassInfo, nullptr, &result.renderPass) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create renderpass");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount                   = static_cast<uint32_t> (shaderStages.size ());
    pipelineInfo.pStages                      = shaderStages.data ();
    pipelineInfo.pVertexInputState            = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState          = &inputAssembly;
    pipelineInfo.pViewportState               = &viewportState;
    pipelineInfo.pRasterizationState          = &rasterizer;
    pipelineInfo.pMultisampleState            = &multisampling;
    pipelineInfo.pDepthStencilState           = nullptr; // Optional
    pipelineInfo.pColorBlendState             = &colorBlending;
    pipelineInfo.pDynamicState                = nullptr; // Optional
    pipelineInfo.layout                       = result.pipelineLayout;
    pipelineInfo.renderPass                   = result.renderPass;
    pipelineInfo.subpass                      = 0;
    pipelineInfo.basePipelineHandle           = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex            = -1;             // Optional

    if (ERROR (vkCreateGraphicsPipelines (device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &result.handle) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create pipeline");
    }

    return result;
}


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


struct Buffer {
    VkBuffer       buffer;
    VkDeviceMemory memory;
};


Buffer CreateBufferMemory (VkPhysicalDevice physicalDevice, VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags)
{
    Buffer result = {};

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size               = bufferSize;
    bufferInfo.usage              = usageFlags;
    bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    if (ERROR (vkCreateBuffer (device, &bufferInfo, nullptr, &result.buffer) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create buffer");
    }

    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements (device, result.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize       = memRequirements.size;
    allocInfo.memoryTypeIndex      = FindMemoryType (physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

    if (ERROR (vkAllocateMemory (device, &allocInfo, nullptr, &result.memory) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to allocate memory");
    }

    if (ERROR (vkBindBufferMemory (device, result.buffer, result.memory, 0) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to bind buffer memory");
    }

    return result;
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


void UploadToHostCoherent (VkDevice device, VkDeviceMemory memory, const void* data, size_t bytes)
{
    void* mappedMemory = nullptr;
    if (ERROR (vkMapMemory (device, memory, 0, bytes, 0, &mappedMemory) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to map memory");
    }
    memcpy (mappedMemory, data, bytes);
    vkUnmapMemory (device, memory);
}

struct UniformBufferObject {
    glm::mat4 m;
    glm::mat4 v;
    glm::mat4 p;
};


void UpdateUniformBuffer (VkDevice device, VkDeviceMemory memory, const Swapchain::SwapchainCreateResult& swapchain, uint32_t currentImage)
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
        ubo.p                   = glm::perspective (glm::radians (45.0f), swapchain.extent.width / static_cast<float> (swapchain.extent.height), 0.1f, 10.0f);
        ubo.p[1][1] *= -1;
        UploadToHostCoherent (device, memory, &ubo, sizeof (ubo));
    }
}


int main (int argc, char* argv[])
{
    std::cout << Utils::PROJECT_ROOT.u8string () << std::endl;

    uint32_t apiVersion;
    vkEnumerateInstanceVersion (&apiVersion);

    WindowProviderUPtr windowProvider = std::make_unique<GLFWWindowProvider> ();

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

    VkDebugUtilsMessengerEXT debugMessenger;
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity                    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType                        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback                    = debugCallback;
        createInfo.pUserData                          = nullptr;

        VkResult result = CreateDebugUtilsMessengerEXT (instance, &createInfo, nullptr, &debugMessenger);
        if (result != VK_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

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

    Buffer stagingBuffer = CreateBufferMemory (
        physicalDevice, device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Buffer vertexBuffer = CreateBufferMemory (
        physicalDevice, device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    UploadToHostCoherent (device, stagingBuffer.memory, vertices.data (), bufferSize);


    ShaderModule vertexShaderModule (device, Utils::PROJECT_ROOT / "shaders" / "shader.vert");
    ShaderModule fragmentShaderModule (device, Utils::PROJECT_ROOT / "shaders" / "shader.frag");

    std::vector<VkPipelineShaderStageCreateInfo>   shaderStages = {vertexShaderModule.GetShaderStageCreateInfo (), fragmentShaderModule.GetShaderStageCreateInfo ()};
    std::vector<VkVertexInputBindingDescription>   vibds        = {Vertex::GetBindingDescription ()};
    std::vector<VkVertexInputAttributeDescription> viads        = Vertex::GetAttributeDescriptions ();


    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding                      = 0;
    uboLayoutBinding.descriptorCount              = 1;
    uboLayoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers           = nullptr;
    uboLayoutBinding.stageFlags                   = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount                    = 1;
    layoutInfo.pBindings                       = &uboLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout (device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error ("failed to create descriptor set layout!");
    }
    std::vector<VkDescriptorSetLayout> layouts (swapchain.imageViews.size (), descriptorSetLayout);


    std::vector<Buffer> uniformBuffers;

    struct PipelineUniforms {
        VkDeviceMemory                   memory;
        void*                            mapped;
        std::vector<Gears::UniformBlock> uniforms;
    };

    const size_t uniformBufferSize = sizeof (UniformBufferObject);
    for (int i = 0; i < swapchain.imageViews.size (); ++i) {
        uniformBuffers.push_back (CreateBufferMemory (
            physicalDevice, device,
            uniformBufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    }

    VkDescriptorPoolSize poolSize       = {};
    poolSize.type                       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount            = static_cast<uint32_t> (swapchain.imageViews.size ());
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount              = 1;
    poolInfo.pPoolSizes                 = &poolSize;
    poolInfo.maxSets                    = static_cast<uint32_t> (swapchain.imageViews.size ());
    VkDescriptorPool descriptorPool;
    if (ERROR (vkCreateDescriptorPool (device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create descriptor pool");
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool              = descriptorPool;
    allocInfo.descriptorSetCount          = static_cast<uint32_t> (swapchain.imageViews.size ());
    allocInfo.pSetLayouts                 = layouts.data ();

    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.resize (swapchain.imageViews.size ());

    if (vkAllocateDescriptorSets (device, &allocInfo, descriptorSets.data ()) != VK_SUCCESS) {
        throw std::runtime_error ("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapchain.imageViews.size (); i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer                 = uniformBuffers[i].buffer;
        bufferInfo.offset                 = 0;
        bufferInfo.range                  = sizeof (UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet               = descriptorSets[i];
        descriptorWrite.dstBinding           = 0;
        descriptorWrite.dstArrayElement      = 0;
        descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount      = 1;
        descriptorWrite.pBufferInfo          = &bufferInfo;
        descriptorWrite.pImageInfo           = nullptr; // Optional
        descriptorWrite.pTexelBufferView     = nullptr; // Optional

        vkUpdateDescriptorSets (device, 1, &descriptorWrite, 0, nullptr);
    }


    PipelineCreateResult graphicsPipeline = CreateGraphicsPipeline (device, swapchain.result, shaderStages, vibds, viads, layouts);
    ASSERT (graphicsPipeline.handle != VK_NULL_HANDLE);

    std::vector<std::unique_ptr<Framebuffer>> swapChainFramebuffers;

    for (size_t i = 0; i < swapchain.imageViews.size (); i++) {
        swapChainFramebuffers.push_back (std::make_unique<Framebuffer> (
            device, graphicsPipeline.renderPass, std::vector<std::reference_wrapper<ImageView>> {*swapchain.imageViews[i]},
            swapchain.result.extent.width, swapchain.result.extent.height));
    }

    VkCommandPool commandPool;

    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex        = *physicalDevice.queueFamilies.graphics;
    commandPoolInfo.flags                   = 0; // Optional
    if (ERROR (vkCreateCommandPool (device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)) {
        return EXIT_FAILURE;
    }

    std::vector<VkCommandBuffer> commandBuffers (swapChainFramebuffers.size ());
    VkCommandBufferAllocateInfo  commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType                        = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool                  = commandPool;
    commandBufferAllocInfo.level                        = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandBufferCount           = (uint32_t)commandBuffers.size ();

    if (ERROR (vkAllocateCommandBuffers (device, &commandBufferAllocInfo, commandBuffers.data ()) != VK_SUCCESS)) {
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < commandBuffers.size (); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = 0;       // Optional
        beginInfo.pInheritanceInfo         = nullptr; // Optional

        if (ERROR (vkBeginCommandBuffer (commandBuffers[i], &beginInfo) != VK_SUCCESS)) {
            return EXIT_FAILURE;
        }

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass            = graphicsPipeline.renderPass;
        renderPassInfo.framebuffer           = *swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset     = {0, 0};
        renderPassInfo.renderArea.extent     = swapchain.result.extent;
        renderPassInfo.clearValueCount       = 1;
        renderPassInfo.pClearValues          = &clearColor;

        vkCmdBeginRenderPass (commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline (commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.handle);

        VkBuffer     vertexBuffers[] = {vertexBuffer.buffer};
        VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers (commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdBindDescriptorSets (commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        vkCmdDraw (commandBuffers[i], 4, 1, 0, 0);

        vkCmdEndRenderPass (commandBuffers[i]);

        if (ERROR (vkEndCommandBuffer (commandBuffers[i]) != VK_SUCCESS)) {
            return EXIT_FAILURE;
        }
    }

    std::vector<VkSemaphore> imageAvailableSemaphore;
    std::vector<VkSemaphore> renderFinishedSemaphore;
    std::vector<VkFence>     inFlightFences;
    std::vector<VkFence>     imagesInFlight;

    imageAvailableSemaphore.resize (MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphore.resize (MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize (MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize (swapchain.imageViews.size ());


    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (ERROR (vkCreateSemaphore (device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
                   vkCreateSemaphore (device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
                   vkCreateFence (device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)) {
            return EXIT_FAILURE;
        }
    }

    size_t currentFrame = 0;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    vkGetDeviceQueue (device, *physicalDevice.queueFamilies.graphics, 0, &graphicsQueue);
    vkGetDeviceQueue (device, *physicalDevice.queueFamilies.presentation, 0, &presentQueue);

    CopyBuffer (device, graphicsQueue, commandPool, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSwapchainKHR       swapChains[] = {swapchain.result.handle};

    std::chrono::time_point<std::chrono::high_resolution_clock> lastDrawTime = std::chrono::high_resolution_clock::now ();

    windowProvider->DoEventLoop ([&] () {
        vkWaitForFences (device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences (device, 1, &inFlightFences[currentFrame]);

        uint32_t imageIndex = 0;
        vkAcquireNextImageKHR (device, swapchain.result.handle, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences (device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        UpdateUniformBuffer (device, uniformBuffers[imageIndex].memory, swapchain.result, imageIndex);

        VkSemaphore waitSemaphores[]   = {imageAvailableSemaphore[currentFrame]};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore[currentFrame]};

        VkSubmitInfo submitInfo         = {};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        vkResetFences (device, 1, &inFlightFences[currentFrame]);

        if (ERROR (vkQueueSubmit (graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)) {
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

    vkDestroyBuffer (device, stagingBuffer.buffer, nullptr);
    vkDestroyBuffer (device, vertexBuffer.buffer, nullptr);

    //vkDestroySemaphore (device, renderFinishedSemaphore, nullptr);
    //vkDestroySemaphore (device, imageAvailableSemaphore, nullptr);

    vkDestroyCommandPool (device, commandPool, nullptr);

    vkDestroyPipeline (device, graphicsPipeline.handle, nullptr);

    vkDestroyRenderPass (device, graphicsPipeline.renderPass, nullptr);

    vkDestroyPipelineLayout (device, graphicsPipeline.pipelineLayout, nullptr);

    DestroyDebugUtilsMessengerEXT (instance, debugMessenger, nullptr);

    return EXIT_SUCCESS;
}