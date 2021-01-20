#ifndef VULKANWRAPPERFWD_HPP
#define VULKANWRAPPERFWD_HPP

#include "Ptr.hpp"

// utils
USING_PTR (BufferTransferable);
USING_PTR (MemoryMapping);
USING_PTR (SingleTimeCommand);
USING_PTR (VulkanObject);

// object wrappers
USING_PTR (Buffer);
USING_PTR (CommandBuffer);
USING_PTR (CommandPool);
USING_PTR (DebugUtilsMessenger);
USING_PTR (DescriptorPool);
USING_PTR (DescriptorSet);
USING_PTR (DescriptorSetLayout);
USING_PTR (DeviceObject);
USING_PTR (DeviceExtra);
USING_PTR (DeviceMemory);
USING_PTR (Fence);
USING_PTR (Framebuffer);
USING_PTR (Image);
USING_PTR (ImageViewBase);
USING_PTR (Instance);
USING_PTR (PhysicalDevice);
USING_PTR (Pipeline);
USING_PTR (PipelineLayout);
USING_PTR (Queue);
USING_PTR (RenderPass);
USING_PTR (Sampler);
USING_PTR (Semaphore);
USING_PTR (ShaderModule);
USING_PTR (ShaderReflection);
USING_PTR (Surface);
USING_PTR (Swapchain);

#endif