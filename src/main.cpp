#include "Assert.hpp"
#include "GLFWWindowProvider.hpp"
#include "Logger.hpp"
#include "ShaderModule.hpp"
#include "Utils.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include <vulkan/vulkan.h>


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

#include <functional>

template<typename SourceType, typename DestType>
static std::set<DestType> ToSet (const std::vector<SourceType>& vec)
{
    std::set<DestType> result;
    std::transform (std::begin (vec), std::end (vec), std::inserter (result, std::end (result)), [] (const SourceType& x) {
        return DestType (x);
    });
    return result;
}

template<typename SourceType, typename DestType>
static std::set<DestType> ToSet (const std::vector<SourceType>& vec, const std::function<DestType (const SourceType&)>& converter)
{
    std::set<DestType> result;
    std::transform (std::begin (vec), std::end (vec), std::inserter (result, std::end (result)), converter);
    return result;
}

template<typename T>
static std::set<T> SetDiff (const std::set<T>& left, const std::set<T>& right)
{
    std::set<T> diff;
    std::set_difference (std::begin (left), std::end (left), std::begin (right), std::end (right), std::inserter (diff, std::end (diff)));
    return diff;
}


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


VkSurfaceFormatKHR ChooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& formats)
{
    if (ERROR (formats.empty ())) {
        return {};
    }

    for (const VkSurfaceFormatKHR& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    ERROR (true);

    return formats[0];
}

VkPresentModeKHR ChooseSwapPresentMode (const std::vector<VkPresentModeKHR>& modes)
{
    if (ERROR (modes.empty ())) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    for (const VkPresentModeKHR& availablePresentMode : modes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    ERROR (true);

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent (const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {800, 600};
        actualExtent.width      = std::clamp (capabilities.minImageExtent.width, capabilities.maxImageExtent.width, actualExtent.width);
        actualExtent.height     = std::clamp (capabilities.minImageExtent.height, capabilities.maxImageExtent.height, actualExtent.height);
        return actualExtent;
    }
}


auto extensionNameAccessor = [] (const VkExtensionProperties& props) { return props.extensionName; };

auto layerNameAccessor = [] (const VkLayerProperties& props) { return props.layerName; };


VkPhysicalDevice CreatePhysicalDevice (VkInstance instance, const std::set<std::string>& requestedDeviceExtensionSet)
{
    // query physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices (instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices (deviceCount);
    vkEnumeratePhysicalDevices (instance, &deviceCount, devices.data ());

    std::optional<uint32_t> physicalDeviceIndex;

    uint32_t i = 0;
    for (VkPhysicalDevice device : devices) {
        // check supported extensions
        uint32_t deviceExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties (device, nullptr, &deviceExtensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedDeviceExtensions (deviceExtensionCount);
        vkEnumerateDeviceExtensionProperties (device, nullptr, &deviceExtensionCount, supportedDeviceExtensions.data ());

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties (device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures (device, &deviceFeatures);

        // check if the device supports all requested extensions
        const std::set<std::string> supportedDeviceExtensionSet   = ToSet<VkExtensionProperties, std::string> (supportedDeviceExtensions, extensionNameAccessor);
        const std::set<std::string> unsupportedDeviceExtensionSet = SetDiff (requestedDeviceExtensionSet, supportedDeviceExtensionSet);

        if (unsupportedDeviceExtensionSet.empty ()) {
            physicalDeviceIndex = i;
            break;
        }

        ++i;
    }

    if (ERROR (!physicalDeviceIndex.has_value ())) {
        throw std::runtime_error ("No physical device available");
    }

    return devices[*physicalDeviceIndex];
}


VkInstance CreateInstance (const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers)
{
    const std::set<std::string> requiredExtensionSet = ToSet<const char*, std::string> (instanceExtensions);

    // supported extensions
    std::set<std::string> supportedExtensionSet;
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions (extensionCount);
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, supportedExtensions.data ());

        supportedExtensionSet = ToSet<VkExtensionProperties, std::string> (supportedExtensions, extensionNameAccessor);
    }

    // check if the required extensions are supported
    {
        const std::set<std::string> unsupportedExtensionSet = SetDiff (requiredExtensionSet, supportedExtensionSet);
        if (ERROR (!unsupportedExtensionSet.empty ())) {
            throw std::runtime_error ("not all instance extensions are supported");
        }
    }


    std::set<std::string> requiredValidationLayerSet = ToSet<const char*, std::string> (instanceLayers);

    // supported validation layers
    std::set<std::string> supportedValidationLayerSet;
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties (&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers (layerCount);
        vkEnumerateInstanceLayerProperties (&layerCount, availableLayers.data ());

        supportedValidationLayerSet = ToSet<VkLayerProperties, std::string> (availableLayers, layerNameAccessor);
    }

    // check if the required validation layers are supported
    {
        const std::set<std::string> unsupportedValidationLayerSet = SetDiff (requiredValidationLayerSet, supportedValidationLayerSet);
        if (ERROR (!unsupportedValidationLayerSet.empty ())) {
            throw std::runtime_error ("not all validation layers are supported");
        }
    }


    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION (1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo    = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = instanceExtensions.size ();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data ();
    createInfo.enabledLayerCount       = static_cast<uint32_t> (instanceLayers.size ());
    createInfo.ppEnabledLayerNames     = instanceLayers.data ();

    VkInstance instance = VK_NULL_HANDLE;
    VkResult   result   = vkCreateInstance (&createInfo, nullptr, &instance);
    if (ERROR (result != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create vulkan instance");
    }

    return instance;
}


VkDevice CreateLogicalDevice (VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex, const std::vector<const char*>& requestedDeviceExtensions)
{
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex        = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount              = 1;
    float queuePriority                     = 1.0f;
    queueCreateInfo.pQueuePriorities        = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo      = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos       = &queueCreateInfo;
    createInfo.queueCreateInfoCount    = 1;
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t> (requestedDeviceExtensions.size ());
    createInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data ();
    createInfo.enabledLayerCount       = 0;

    VkDevice device = VK_NULL_HANDLE;
    vkCreateDevice (physicalDevice, &createInfo, nullptr, &device);
    return device;
}


struct QueueFamilies {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> presentation;

    bool IsValid () const
    {
        return graphics.has_value () && presentation.has_value ();
    }
};


QueueFamilies CreateQueueFamilyIndices (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilies result;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, queueFamilies.data ());


    int i = 0;
    for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            result.graphics = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR (physicalDevice, i, surface, &presentSupport);
        if (presentSupport) {
            result.presentation = i;
        }

        if (result.IsValid ()) {
            break;
        }

        i++;
    }

    if (ERROR (!result.IsValid ())) {
        throw std::runtime_error ("failed to find queue family indices");
    }

    return result;
}


struct SwapchainCreateResult {
    VkSwapchainKHR     handle;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR   presentMode;
    VkExtent2D         extent;
};


SwapchainCreateResult CreateSwapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueFamilies queueFamilyIndices)
{
    if (ERROR (!queueFamilyIndices.IsValid ())) {
        throw std::runtime_error ("bad indices");
    }

    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physicalDevice, surface, &details.capabilities);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR (physicalDevice, surface, &formatCount, nullptr);
    details.formats.resize (formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR (physicalDevice, surface, &formatCount, details.formats.data ());
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR (physicalDevice, surface, &presentModeCount, nullptr);
    details.presentModes.resize (presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR (physicalDevice, surface, &presentModeCount, details.presentModes.data ());

    SwapchainCreateResult result;

    result.surfaceFormat = ChooseSwapSurfaceFormat (details.formats);
    result.presentMode   = ChooseSwapPresentMode (details.presentModes);
    result.extent        = ChooseSwapExtent (details.capabilities);

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = surface;
    createInfo.minImageCount            = imageCount;
    createInfo.imageFormat              = result.surfaceFormat.format;
    createInfo.imageColorSpace          = result.surfaceFormat.colorSpace;
    createInfo.imageExtent              = result.extent;
    createInfo.imageArrayLayers         = 1;
    createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndicesData[] = {*queueFamilyIndices.graphics, *queueFamilyIndices.presentation};
    if (ERROR (*queueFamilyIndices.graphics != *queueFamilyIndices.presentation)) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndicesData;
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;       // Optional
        createInfo.pQueueFamilyIndices   = nullptr; // Optional
    }
    createInfo.preTransform   = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = result.presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    if (ERROR (vkCreateSwapchainKHR (device, &createInfo, nullptr, &result.handle) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create swapchain");
    }

    return result;
}


std::vector<VkImageView> CreateSwapchainImageViews (VkDevice device, SwapchainCreateResult swapchain)
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR (device, swapchain.handle, &imageCount, nullptr);
    std::vector<VkImage> swapChainImages (imageCount);
    vkGetSwapchainImagesKHR (device, swapchain.handle, &imageCount, swapChainImages.data ());
    std::vector<VkImageView> swapChainImageViews (imageCount);

    for (size_t i = 0; i < swapChainImages.size (); ++i) {
        VkImageViewCreateInfo createInfo           = {};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                           = swapChainImages[i];
        createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                          = swapchain.surfaceFormat.format;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;
        if (ERROR (vkCreateImageView (device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create swapchain image views");
        }
    }

    return swapChainImageViews;
}


struct PipelineCreateResult {
    VkPipeline       handle;
    VkRenderPass     renderPass;
    VkPipelineLayout pipelineLayout;
};


PipelineCreateResult CreateGraphicsPipeline (
    VkDevice                                              device,
    SwapchainCreateResult                                 swapchain,
    const std::vector<VkPipelineShaderStageCreateInfo>&   shaderStages,
    const std::vector<VkVertexInputBindingDescription>&   vertexBindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions)
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
    rasterizer.frontFace                                     = VK_FRONT_FACE_CLOCKWISE;
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
    pipelineLayoutInfo.setLayoutCount                        = 0;       // Optional
    pipelineLayoutInfo.pSetLayouts                           = nullptr; // Optional
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


VkShaderModule CreateShaderModule (VkDevice device, const std::filesystem::path& binaryFilePath)
{
    std::optional<std::vector<char>> bytecode = Utils::ReadBinaryFile (binaryFilePath);
    if (ERROR (!bytecode.has_value ())) {
        throw std::runtime_error ("failed to read shader bytecode");
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = static_cast<uint32_t> (bytecode->size ());
    createInfo.pCode                    = reinterpret_cast<const uint32_t*> (bytecode->data ());

    VkShaderModule result = VK_NULL_HANDLE;
    if (ERROR (vkCreateShaderModule (device, &createInfo, nullptr, &result) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create shader module");
    }

    return result;
}

#include <glm/glm.hpp>


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


VkBuffer CreateBuffer (VkDevice device, size_t size)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size               = size;
    bufferInfo.usage              = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer handle;
    if (ERROR (vkCreateBuffer (device, &bufferInfo, nullptr, &handle) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create vertex buffer");
    }

    return handle;
}

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

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    const VkInstance instance = CreateInstance (extensions, validationLayers);
    ASSERT (instance != VK_NULL_HANDLE);

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

    const VkSurfaceKHR surface = windowProvider->CreateSurface (instance);
    ASSERT (surface != VK_NULL_HANDLE);

    const std::vector<const char*> requestedDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const std::set<std::string> requestedDeviceExtensionSet = ToSet<const char*, std::string> (requestedDeviceExtensions);

    const VkPhysicalDevice physicalDevice = CreatePhysicalDevice (instance, requestedDeviceExtensionSet);

    const QueueFamilies queueFamilyIndices = CreateQueueFamilyIndices (physicalDevice, surface);

    VkDevice device = CreateLogicalDevice (physicalDevice, *queueFamilyIndices.graphics, requestedDeviceExtensions);
    ASSERT (device != VK_NULL_HANDLE);

    const std::vector<Vertex> vertices = {
        Vertex {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        Vertex {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        Vertex {{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
    };

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    {
        const size_t vertexBufferSize = vertices.size () * sizeof (Vertex);
        vertexBuffer                  = CreateBuffer (device, vertexBufferSize);

        VkMemoryRequirements memRequirements = {};
        vkGetBufferMemoryRequirements (device, vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize       = memRequirements.size;
        allocInfo.memoryTypeIndex      = FindMemoryType (physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDeviceMemory vertexBufferMemory;
        if (vkAllocateMemory (device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
            throw std::runtime_error ("failed to allocate vertex buffer memory!");
        }

        VkResult result = vkBindBufferMemory (device, vertexBuffer, vertexBufferMemory, 0);
        if (result != VK_SUCCESS) {
            throw std::runtime_error ("failed to bind memory!");
        }

        void* data;
        vkMapMemory (device, vertexBufferMemory, 0, vertexBufferSize, 0, &data);
        memcpy (data, vertices.data (), vertexBufferSize);
        vkUnmapMemory (device, vertexBufferMemory);
    }

    SwapchainCreateResult swapchain = CreateSwapchain (physicalDevice, device, surface, queueFamilyIndices);
    ASSERT (swapchain.handle != VK_NULL_HANDLE);

    std::vector<VkImageView> swapChainImageViews = CreateSwapchainImageViews (device, swapchain);

    VkShaderModule vertexShaderModule = CreateShaderModule (device, Utils::PROJECT_ROOT / "src" / "shader.vert.spv");
    ASSERT (vertexShaderModule != VK_NULL_HANDLE);

    VkShaderModule fragmentShaderModule = CreateShaderModule (device, Utils::PROJECT_ROOT / "src" / "shader.frag.spv");
    ASSERT (fragmentShaderModule != VK_NULL_HANDLE);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo       = {};
    vertShaderStageInfo.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage                                 = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module                                = vertexShaderModule;
    vertShaderStageInfo.pName                                 = "main";
    VkPipelineShaderStageCreateInfo fragShaderStageInfo       = {};
    fragShaderStageInfo.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage                                 = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module                                = fragmentShaderModule;
    fragShaderStageInfo.pName                                 = "main";
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

    std::vector<VkVertexInputBindingDescription>   vibds = {Vertex::GetBindingDescription ()};
    std::vector<VkVertexInputAttributeDescription> viads = Vertex::GetAttributeDescriptions ();

    PipelineCreateResult graphicsPipeline = CreateGraphicsPipeline (device, swapchain, shaderStages, vibds, viads);
    ASSERT (graphicsPipeline.handle != VK_NULL_HANDLE);

    std::vector<VkFramebuffer> swapChainFramebuffers (swapChainImageViews.size ());
    for (size_t i = 0; i < swapChainImageViews.size (); i++) {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass              = graphicsPipeline.renderPass;
        framebufferInfo.attachmentCount         = 1;
        framebufferInfo.pAttachments            = attachments;
        framebufferInfo.width                   = swapchain.extent.width;
        framebufferInfo.height                  = swapchain.extent.height;
        framebufferInfo.layers                  = 1;

        if (ERROR (vkCreateFramebuffer (device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)) {
            return EXIT_FAILURE;
        }
    }

    VkCommandPool commandPool;

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex        = *queueFamilyIndices.graphics;
    poolInfo.flags                   = 0; // Optional
    if (ERROR (vkCreateCommandPool (device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)) {
        return EXIT_FAILURE;
    }

    std::vector<VkCommandBuffer> commandBuffers (swapChainFramebuffers.size ());
    VkCommandBufferAllocateInfo  allocInfo = {};
    allocInfo.sType                        = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool                  = commandPool;
    allocInfo.level                        = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount           = (uint32_t)commandBuffers.size ();

    if (ERROR (vkAllocateCommandBuffers (device, &allocInfo, commandBuffers.data ()) != VK_SUCCESS)) {
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
        renderPassInfo.framebuffer           = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset     = {0, 0};
        renderPassInfo.renderArea.extent     = swapchain.extent;
        renderPassInfo.clearValueCount       = 1;
        renderPassInfo.pClearValues          = &clearColor;

        vkCmdBeginRenderPass (commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline (commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.handle);

        VkBuffer     vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers (commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdDraw (commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass (commandBuffers[i]);

        if (ERROR (vkEndCommandBuffer (commandBuffers[i]) != VK_SUCCESS)) {
            return EXIT_FAILURE;
        }
    }

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (ERROR (vkCreateSemaphore (device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
               vkCreateSemaphore (device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)) {
        return EXIT_FAILURE;
    }


    VkQueue graphicsQueue;
    VkQueue presentQueue;
    vkGetDeviceQueue (device, *queueFamilyIndices.graphics, 0, &graphicsQueue);
    vkGetDeviceQueue (device, *queueFamilyIndices.presentation, 0, &presentQueue);

    VkSemaphore          waitSemaphores[]   = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[]       = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore          signalSemaphores[] = {renderFinishedSemaphore};
    VkSwapchainKHR       swapChains[]       = {swapchain.handle};

    windowProvider->DoEventLoop ([&] () {
        uint32_t imageIndex = 0;
        vkAcquireNextImageKHR (device, swapchain.handle, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        VkSubmitInfo submitInfo         = {};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        if (ERROR (vkQueueSubmit (graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)) {
            return;
        }


        VkPresentInfoKHR presentInfo   = {};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = swapChains;
        presentInfo.pImageIndices      = &imageIndex;
        presentInfo.pResults           = nullptr; // Optional

        vkQueuePresentKHR (presentQueue, &presentInfo);

        vkQueueWaitIdle (presentQueue);
    });

    vkDeviceWaitIdle (device);

    vkDestroyShaderModule (device, vertexShaderModule, nullptr);
    vkDestroyShaderModule (device, fragmentShaderModule, nullptr);

    for (VkFramebuffer framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer (device, framebuffer, nullptr);
    }


    vkDestroySemaphore (device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore (device, imageAvailableSemaphore, nullptr);

    vkDestroyCommandPool (device, commandPool, nullptr);

    vkDestroyPipeline (device, graphicsPipeline.handle, nullptr);

    vkDestroyRenderPass (device, graphicsPipeline.renderPass, nullptr);

    vkDestroyPipelineLayout (device, graphicsPipeline.pipelineLayout, nullptr);

    for (VkImageView imageView : swapChainImageViews) {
        vkDestroyImageView (device, imageView, nullptr);
    }

    vkDestroySwapchainKHR (device, swapchain.handle, nullptr);

    vkDestroyDevice (device, nullptr);

    vkDestroySurfaceKHR (instance, surface, nullptr);

    DestroyDebugUtilsMessengerEXT (instance, debugMessenger, nullptr);

    vkDestroyInstance (instance, nullptr);

    return EXIT_SUCCESS;
}