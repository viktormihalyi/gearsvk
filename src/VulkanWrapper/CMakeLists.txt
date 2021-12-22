add_library (VulkanWrapper SHARED)

set (Headers
    Include/VulkanWrapper/Utils/BufferTransferable.hpp
    Include/VulkanWrapper/Utils/ImageData.hpp
    Include/VulkanWrapper/Utils/MemoryMapping.hpp
    Include/VulkanWrapper/Utils/SingleTimeCommand.hpp
    Include/VulkanWrapper/Utils/VulkanUtils.hpp

    Include/VulkanWrapper/Allocator.hpp
    Include/VulkanWrapper/Buffer.hpp
    Include/VulkanWrapper/Command.hpp
    Include/VulkanWrapper/CommandBuffer.hpp
    Include/VulkanWrapper/CommandPool.hpp
    Include/VulkanWrapper/Commands.hpp
    Include/VulkanWrapper/ComputePipeline.hpp
    Include/VulkanWrapper/PipelineBase.hpp
    Include/VulkanWrapper/DebugReportCallback.hpp
    Include/VulkanWrapper/DebugUtilsMessenger.hpp
    Include/VulkanWrapper/DescriptorPool.hpp
    Include/VulkanWrapper/DescriptorSet.hpp
    Include/VulkanWrapper/DescriptorSetLayout.hpp
    Include/VulkanWrapper/Device.hpp
    Include/VulkanWrapper/DeviceExtra.hpp
    Include/VulkanWrapper/DeviceMemory.hpp
    Include/VulkanWrapper/Event.hpp
    Include/VulkanWrapper/Fence.hpp
    Include/VulkanWrapper/Framebuffer.hpp
    Include/VulkanWrapper/GraphicsPipeline.hpp
    Include/VulkanWrapper/Image.hpp
    Include/VulkanWrapper/ImageView.hpp
    Include/VulkanWrapper/Instance.hpp
    Include/VulkanWrapper/PhysicalDevice.hpp
    Include/VulkanWrapper/PipelineLayout.hpp
    Include/VulkanWrapper/Queue.hpp
    Include/VulkanWrapper/RenderPass.hpp
    Include/VulkanWrapper/Sampler.hpp
    Include/VulkanWrapper/Semaphore.hpp
    Include/VulkanWrapper/ShaderModule.hpp
    Include/VulkanWrapper/ShaderReflection.hpp
    Include/VulkanWrapper/Surface.hpp
    Include/VulkanWrapper/Swapchain.hpp
    Include/VulkanWrapper/VulkanObject.hpp
    Include/VulkanWrapper/VulkanWrapper.hpp
    Include/VulkanWrapper/VulkanWrapperFwd.hpp

    Include/VulkanWrapper/VulkanWrapperAPI.hpp
)

set (SourcesGroup1
    Sources/Utils/BufferTransferable.cpp
    Sources/Utils/ImageData.cpp
    Sources/Utils/MemoryMapping.cpp
    Sources/Utils/VulkanUtils.cpp

    Sources/Allocator.cpp
    Sources/Buffer.cpp
    Sources/CommandBuffer.cpp
    Sources/Commands.cpp
    Sources/ComputePipeline.cpp
    Sources/PipelineBase.cpp
    Sources/DebugReportCallback.cpp
    Sources/DebugUtilsMessenger.cpp
    Sources/Device.cpp
    Sources/DeviceMemory.cpp
    Sources/Event.cpp
    Sources/Fence.cpp
    Sources/Framebuffer.cpp
    Sources/GraphicsPipeline.cpp
    Sources/Image.cpp
    Sources/ImageView.cpp
    Sources/Instance.cpp
    Sources/PhysicalDevice.cpp
    Sources/Queue.cpp
    Sources/ResourceLimits.cpp
    Sources/Sampler.cpp
    Sources/ShaderModule.cpp
    Sources/ShaderReflection.cpp
    Sources/Surface.cpp
    Sources/Swapchain.cpp
    Sources/VulkanObject.cpp
    Sources/LogInitializer.cpp
)
set (SourcesGroup2 Sources/Utils/STBImpl.cpp)
set (SourcesGroup3 Sources/VulkanMemoryAllocatorImpl.cpp)

set (Sources ${SourcesGroup1} ${SourcesGroup2} ${SourcesGroup3})

target_sources (VulkanWrapper PRIVATE ${Headers} ${Sources})

find_package (Vulkan REQUIRED)
find_package (vulkan-memory-allocator REQUIRED)
find_package (fmt REQUIRED)
find_package (spdlog REQUIRED)
find_package (stb REQUIRED)
find_package (glslang REQUIRED)
find_package (spirv-cross REQUIRED)

set_target_properties (VulkanWrapper PROPERTIES UNITY_BUILD_MODE GROUP)
set_source_files_properties (${SourcesGroup1} PROPERTIES UNITY_GROUP "bucket1")
set_source_files_properties (${SourcesGroup2} PROPERTIES UNITY_GROUP "bucket2")
set_source_files_properties (${SourcesGroup3} PROPERTIES UNITY_GROUP "bucket3")

target_compile_definitions (VulkanWrapper
    PUBLIC SPDLOG_FMT_EXTERNAL
)

target_precompile_headers (VulkanWrapper PRIVATE PrecompiledHeader.hpp)

target_include_directories (VulkanWrapper
    PRIVATE
        Include
        Include/VulkanWrapper
        Include/VulkanWrapper/Utils
        ${fmt_INCLUDE_DIRS}
        ${spdlog_INCLUDE_DIRS}
        ${stb_INCLUDE_DIRS}
        ${glslang_INCLUDE_DIRS}
        ${spirv-cross_INCLUDE_DIRS}
    
    PUBLIC
        Include
        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
        ${Vulkan_INCLUDE_DIRS}
        ${vulkan-memory-allocator_INCLUDE_DIRS}
)

target_link_libraries (VulkanWrapper
    PRIVATE
        ${fmt_LIBRARIES}
        ${spdlog_LIBRARIES}
        ${stb_LIBRARIES}
        ${glslang_LIBRARIES}
        ${spirv-cross_LIBRARIES}

    PUBLIC
        ${Vulkan_LIBRARIES}
        ${vulkan-memory-allocator_LIBRARIES}
        Utils
)
