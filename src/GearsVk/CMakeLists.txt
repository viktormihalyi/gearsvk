
set (IncludePath ${CMAKE_SOURCE_DIR}/src/GearsVk/Include)
set (HeadersPath ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk)
set (Headers
    ${HeadersPath}/Camera/Camera.hpp
    ${HeadersPath}/Camera/CameraControl.hpp
    ${HeadersPath}/Camera/Frustum.hpp

    ${HeadersPath}/RenderGraph/DrawRecordable/DrawRecordable.hpp
    ${HeadersPath}/RenderGraph/DrawRecordable/DrawRecordableInfo.hpp
    ${HeadersPath}/RenderGraph/DrawRecordable/FullscreenQuad.hpp

    ${HeadersPath}/RenderGraph/Connections.hpp
    ${HeadersPath}/RenderGraph/GraphRenderer.hpp
    ${HeadersPath}/RenderGraph/GraphSettings.hpp
    ${HeadersPath}/RenderGraph/InputBindable.hpp
    ${HeadersPath}/RenderGraph/Node.hpp
    ${HeadersPath}/RenderGraph/Operation.hpp
    ${HeadersPath}/RenderGraph/RenderGraph.hpp
    ${HeadersPath}/RenderGraph/Resource.hpp
    ${HeadersPath}/RenderGraph/ShaderPipeline.hpp
    ${HeadersPath}/RenderGraph/UniformReflection.hpp
    
    ${HeadersPath}/VulkanWrapper/Utils/BufferTransferable.hpp
    ${HeadersPath}/VulkanWrapper/Utils/ImageData.hpp
    ${HeadersPath}/VulkanWrapper/Utils/MemoryMapping.hpp
    ${HeadersPath}/VulkanWrapper/Utils/SingleTimeCommand.hpp
    ${HeadersPath}/VulkanWrapper/Utils/VulkanUtils.hpp

    ${HeadersPath}/VulkanWrapper/Allocator.hpp
    ${HeadersPath}/VulkanWrapper/Buffer.hpp
    ${HeadersPath}/VulkanWrapper/CommandBuffer.hpp
    ${HeadersPath}/VulkanWrapper/CommandPool.hpp
    ${HeadersPath}/VulkanWrapper/DebugUtilsMessenger.hpp
    ${HeadersPath}/VulkanWrapper/DescriptorPool.hpp
    ${HeadersPath}/VulkanWrapper/DescriptorSet.hpp
    ${HeadersPath}/VulkanWrapper/DescriptorSetLayout.hpp
    ${HeadersPath}/VulkanWrapper/Device.hpp
    ${HeadersPath}/VulkanWrapper/DeviceExtra.hpp
    ${HeadersPath}/VulkanWrapper/DeviceMemory.hpp
    ${HeadersPath}/VulkanWrapper/Fence.hpp
    ${HeadersPath}/VulkanWrapper/Framebuffer.hpp
    ${HeadersPath}/VulkanWrapper/Image.hpp
    ${HeadersPath}/VulkanWrapper/ImageView.hpp
    ${HeadersPath}/VulkanWrapper/Instance.hpp
    ${HeadersPath}/VulkanWrapper/PhysicalDevice.hpp
    ${HeadersPath}/VulkanWrapper/Pipeline.hpp
    ${HeadersPath}/VulkanWrapper/PipelineLayout.hpp
    ${HeadersPath}/VulkanWrapper/Queue.hpp
    ${HeadersPath}/VulkanWrapper/RenderPass.hpp
    ${HeadersPath}/VulkanWrapper/ResourceLimits.hpp
    ${HeadersPath}/VulkanWrapper/Sampler.hpp
    ${HeadersPath}/VulkanWrapper/Semaphore.hpp
    ${HeadersPath}/VulkanWrapper/ShaderModule.hpp
    ${HeadersPath}/VulkanWrapper/ShaderReflection.hpp
    ${HeadersPath}/VulkanWrapper/Surface.hpp
    ${HeadersPath}/VulkanWrapper/Swapchain.hpp
    ${HeadersPath}/VulkanWrapper/VulkanObject.hpp
    ${HeadersPath}/VulkanWrapper/VulkanWrapper.hpp
    ${HeadersPath}/VulkanWrapper/VulkanWrapperFwd.hpp

    ${HeadersPath}/Window/GLFWWindow.hpp
    ${HeadersPath}/Window/SDLWindow.hpp
    ${HeadersPath}/Window/Window.hpp

    ${HeadersPath}/Font.hpp
    ${HeadersPath}/GearsVkAPI.hpp
    ${HeadersPath}/glmlib.hpp
    ${HeadersPath}/InputState.hpp
    ${HeadersPath}/UniformView.hpp
    ${HeadersPath}/VulkanEnvironment.hpp
)

set (SourcesPath ${CMAKE_SOURCE_DIR}/src/GearsVk/Sources)
set (Sources
    ${SourcesPath}/Camera/Camera.cpp
    ${SourcesPath}/Camera/Frustum.cpp

    ${SourcesPath}/RenderGraph/GraphRenderer.cpp
    ${SourcesPath}/RenderGraph/GraphSettings.cpp
    ${SourcesPath}/RenderGraph/Operation.cpp
    ${SourcesPath}/RenderGraph/RenderGraph.cpp
    ${SourcesPath}/RenderGraph/Resource.cpp
    ${SourcesPath}/RenderGraph/ShaderPipeline.cpp
    ${SourcesPath}/RenderGraph/UniformReflection.cpp
    
    ${SourcesPath}/VulkanWrapper/Utils/BufferTransferable.cpp
    ${SourcesPath}/VulkanWrapper/Utils/ImageData.cpp
    ${SourcesPath}/VulkanWrapper/Utils/STBImpl.cpp
    ${SourcesPath}/VulkanWrapper/Utils/VulkanUtils.cpp

    ${SourcesPath}/VulkanWrapper/Allocator.cpp
    ${SourcesPath}/VulkanWrapper/Buffer.cpp
    ${SourcesPath}/VulkanWrapper/CommandBuffer.cpp
    ${SourcesPath}/VulkanWrapper/DebugUtilsMessenger.cpp
    ${SourcesPath}/VulkanWrapper/Device.cpp
    ${SourcesPath}/VulkanWrapper/DeviceMemory.cpp
    ${SourcesPath}/VulkanWrapper/Fence.cpp
    ${SourcesPath}/VulkanWrapper/Framebuffer.cpp
    ${SourcesPath}/VulkanWrapper/Image.cpp
    ${SourcesPath}/VulkanWrapper/ImageView.cpp
    ${SourcesPath}/VulkanWrapper/Instance.cpp
    ${SourcesPath}/VulkanWrapper/PhysicalDevice.cpp
    ${SourcesPath}/VulkanWrapper/Pipeline.cpp
    ${SourcesPath}/VulkanWrapper/Queue.cpp
    ${SourcesPath}/VulkanWrapper/ResourceLimits.cpp
    ${SourcesPath}/VulkanWrapper/Sampler.cpp
    ${SourcesPath}/VulkanWrapper/ShaderModule.cpp
    ${SourcesPath}/VulkanWrapper/ShaderReflection.cpp
    ${SourcesPath}/VulkanWrapper/Surface.cpp
    ${SourcesPath}/VulkanWrapper/Swapchain.cpp
    ${SourcesPath}/VulkanWrapper/VulkanObject.cpp

    ${SourcesPath}/Window/GLFWWindow.cpp
    ${SourcesPath}/Window/SDLWindow.cpp

    #${SourcesPath}/Font.cpp
    ${SourcesPath}/SpdlogInitializer.cpp
    ${SourcesPath}/UniformView.cpp
    ${SourcesPath}/VulkanEnvironment.cpp
    ${SourcesPath}/VulkanMemoryAllocatorImpl.cpp
)

add_library (RenderGraph SHARED ${Headers} ${Sources})

find_package (Vulkan REQUIRED)
find_package (glfw3 REQUIRED)
find_package (glm REQUIRED)
find_package (vulkan-memory-allocator REQUIRED)
find_package (fmt REQUIRED)
find_package (spdlog REQUIRED)
find_package (stb REQUIRED)
find_package (glslang REQUIRED)
find_package (spirv-cross REQUIRED)

target_compile_definitions (RenderGraph PRIVATE SPDLOG_FMT_EXTERNAL)

target_precompile_headers (RenderGraph PRIVATE [["PCH.hpp"]])

target_include_directories (RenderGraph
    PRIVATE ${HeadersPath}
    PRIVATE ${HeadersPath}/Camera
    PRIVATE ${HeadersPath}/RenderGraph
    PRIVATE ${HeadersPath}/RenderGraph/DrawRecordable
    PRIVATE ${HeadersPath}/VulkanWrapper
    PRIVATE ${HeadersPath}/VulkanWrapper/Utils
    PRIVATE ${HeadersPath}/Window
    
    PUBLIC  ${IncludePath}

    PUBLIC  ${CMAKE_SOURCE_DIR}/src/Utils/Include
    
    PUBLIC  ${Vulkan_INCLUDE_DIRS}
    PUBLIC  ${glm_INCLUDE_DIRS}
    PUBLIC  ${glfw_INCLUDE_DIRS}
    PUBLIC  ${vulkan-memory-allocator_INCLUDE_DIRS}
    PUBLIC  ${fmt_INCLUDE_DIRS}
    PUBLIC  ${spdlog_INCLUDE_DIRS}
    PUBLIC  ${stb_INCLUDE_DIRS}
    PUBLIC  ${glslang_INCLUDE_DIRS}
    PUBLIC  ${spirv-cross_INCLUDE_DIRS}
)

target_link_libraries (RenderGraph
    PUBLIC ${Vulkan_LIBRARIES}
    PUBLIC ${glm_LIBRARIES}
    PUBLIC ${glfw_LIBRARIES}
    PUBLIC ${vulkan-memory-allocator_LIBRARIES}
    PUBLIC ${fmt_LIBRARIES}
    PUBLIC ${spdlog_LIBRARIES}
    PUBLIC ${stb_LIBRARIES}
    PUBLIC ${glslang_LIBRARIES}
    PUBLIC ${spirv-cross_LIBRARIES}
    PUBLIC Utils
)

install (TARGETS RenderGraph
         RUNTIME DESTINATION bin)
