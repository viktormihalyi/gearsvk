
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanWrapper)
set (Headers
    ${HeadersPath}/Utils/BufferTransferable.hpp
    ${HeadersPath}/Utils/ImageData.hpp
    ${HeadersPath}/Utils/MemoryMapping.hpp
    ${HeadersPath}/Utils/SingleTimeCommand.hpp
    ${HeadersPath}/Utils/VulkanUtils.hpp

    ${HeadersPath}/Allocator.hpp
    ${HeadersPath}/Buffer.hpp
    ${HeadersPath}/Command.hpp
    ${HeadersPath}/Commands.hpp
    ${HeadersPath}/CommandBuffer.hpp
    ${HeadersPath}/CommandPool.hpp
    ${HeadersPath}/DebugReportCallback.hpp
    ${HeadersPath}/DebugUtilsMessenger.hpp
    ${HeadersPath}/DescriptorPool.hpp
    ${HeadersPath}/DescriptorSet.hpp
    ${HeadersPath}/DescriptorSetLayout.hpp
    ${HeadersPath}/Device.hpp
    ${HeadersPath}/DeviceExtra.hpp
    ${HeadersPath}/DeviceMemory.hpp
    ${HeadersPath}/Fence.hpp
    ${HeadersPath}/Framebuffer.hpp
    ${HeadersPath}/Image.hpp
    ${HeadersPath}/ImageView.hpp
    ${HeadersPath}/Instance.hpp
    ${HeadersPath}/PhysicalDevice.hpp
    ${HeadersPath}/Pipeline.hpp
    ${HeadersPath}/PipelineLayout.hpp
    ${HeadersPath}/Queue.hpp
    ${HeadersPath}/RenderPass.hpp
    ${HeadersPath}/ResourceLimits.hpp
    ${HeadersPath}/Sampler.hpp
    ${HeadersPath}/Semaphore.hpp
    ${HeadersPath}/ShaderModule.hpp
    ${HeadersPath}/ShaderReflection.hpp
    ${HeadersPath}/Surface.hpp
    ${HeadersPath}/Swapchain.hpp
    ${HeadersPath}/VulkanObject.hpp
    ${HeadersPath}/VulkanWrapper.hpp
    ${HeadersPath}/VulkanWrapperFwd.hpp

    ${HeadersPath}/VulkanWrapperAPI.hpp
)

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (SourcesGroup1
    ${SourcesPath}/Utils/BufferTransferable.cpp
    ${SourcesPath}/Utils/ImageData.cpp
    ${SourcesPath}/Utils/MemoryMapping.cpp
    ${SourcesPath}/Utils/VulkanUtils.cpp

    ${SourcesPath}/Allocator.cpp
    ${SourcesPath}/Buffer.cpp
    ${SourcesPath}/Commands.cpp
    ${SourcesPath}/CommandBuffer.cpp
    ${SourcesPath}/DebugReportCallback.cpp
    ${SourcesPath}/DebugUtilsMessenger.cpp
    ${SourcesPath}/Device.cpp
    ${SourcesPath}/DeviceMemory.cpp
    ${SourcesPath}/Fence.cpp
    ${SourcesPath}/Framebuffer.cpp
    ${SourcesPath}/Image.cpp
    ${SourcesPath}/ImageView.cpp
    ${SourcesPath}/Instance.cpp
    ${SourcesPath}/PhysicalDevice.cpp
    ${SourcesPath}/Pipeline.cpp
    ${SourcesPath}/Queue.cpp
    ${SourcesPath}/ResourceLimits.cpp
    ${SourcesPath}/Sampler.cpp
    ${SourcesPath}/ShaderModule.cpp
    ${SourcesPath}/ShaderReflection.cpp
    ${SourcesPath}/Surface.cpp
    ${SourcesPath}/Swapchain.cpp
    ${SourcesPath}/VulkanObject.cpp
)
set (SourcesGroup2 ${SourcesPath}/Utils/STBImpl.cpp)
set (SourcesGroup3 ${SourcesPath}/VulkanMemoryAllocatorImpl.cpp)

set (Sources ${SourcesGroup1} ${SourcesGroup2} ${SourcesGroup3})

add_library (VulkanWrapper SHARED ${Headers} ${Sources})

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

target_include_directories (VulkanWrapper
    PRIVATE ${IncludePath}
    PRIVATE ${HeadersPath}
    PRIVATE ${HeadersPath}/Utils
    
    PUBLIC ${IncludePath}
    PUBLIC $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
    
    PUBLIC ${Vulkan_INCLUDE_DIRS}
    PUBLIC ${vulkan-memory-allocator_INCLUDE_DIRS}
    PRIVATE ${fmt_INCLUDE_DIRS}
    PRIVATE ${spdlog_INCLUDE_DIRS}
    PRIVATE ${stb_INCLUDE_DIRS}
    PRIVATE ${glslang_INCLUDE_DIRS}
    PRIVATE ${spirv-cross_INCLUDE_DIRS}
)

target_link_libraries (VulkanWrapper
    PUBLIC ${Vulkan_LIBRARIES}
    PUBLIC ${vulkan-memory-allocator_LIBRARIES}
    PRIVATE ${fmt_LIBRARIES}
    PRIVATE ${spdlog_LIBRARIES}
    PRIVATE ${stb_LIBRARIES}
    PRIVATE ${glslang_LIBRARIES}
    PRIVATE ${spirv-cross_LIBRARIES}
    PUBLIC Utils
)
