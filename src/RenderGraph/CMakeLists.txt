
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/RenderGraph)
set (Headers
    ${HeadersPath}/DrawRecordable/DrawRecordable.hpp
    ${HeadersPath}/DrawRecordable/DrawRecordableInfo.hpp
    ${HeadersPath}/DrawRecordable/FullscreenQuad.hpp

    ${HeadersPath}/GraphRenderer.hpp
    ${HeadersPath}/GraphSettings.hpp
    ${HeadersPath}/InputBindable.hpp
    ${HeadersPath}/Node.hpp
    ${HeadersPath}/Operation.hpp
    ${HeadersPath}/RenderGraph.hpp
    ${HeadersPath}/RenderGraphPass.hpp
    ${HeadersPath}/Resource.hpp
    ${HeadersPath}/ShaderPipeline.hpp
    ${HeadersPath}/UniformReflection.hpp
    
    ${HeadersPath}/Window/GLFWWindow.hpp
    ${HeadersPath}/Window/SDLWindow.hpp
    ${HeadersPath}/Window/Window.hpp

    ${HeadersPath}/Font.hpp
    ${HeadersPath}/RenderGraphAPI.hpp
    ${HeadersPath}/UniformView.hpp
    ${HeadersPath}/VulkanEnvironment.hpp

    ${HeadersPath}/ShaderReflectionToVertexAttribute.hpp
    ${HeadersPath}/ShaderReflectionToDescriptor.hpp
    ${HeadersPath}/ShaderReflectionToAttachment.hpp

)

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/DrawRecordable/DrawRecordableInfo.cpp

    ${SourcesPath}/GraphRenderer.cpp
    ${SourcesPath}/GraphSettings.cpp
    ${SourcesPath}/Operation.cpp
    ${SourcesPath}/RenderGraph.cpp
    ${SourcesPath}/RenderGraphPass.cpp
    ${SourcesPath}/Resource.cpp
    ${SourcesPath}/ShaderPipeline.cpp
    ${SourcesPath}/UniformReflection.cpp

    ${SourcesPath}/Window/GLFWWindow.cpp
    ${SourcesPath}/Window/SDLWindow.cpp

    ${SourcesPath}/Font.cpp
    ${SourcesPath}/SpdlogInitializer.cpp
    ${SourcesPath}/UniformView.cpp
    ${SourcesPath}/VulkanEnvironment.cpp
    
    ${SourcesPath}/ShaderReflectionToVertexAttribute.cpp
    ${SourcesPath}/ShaderReflectionToDescriptor.cpp
    ${SourcesPath}/ShaderReflectionToAttachment.cpp
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
find_package (msdfgen REQUIRED)

target_compile_definitions (RenderGraph
    PUBLIC SPDLOG_FMT_EXTERNAL
    PUBLIC GLM_FORCE_RADIANS
    PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE
)

target_include_directories (RenderGraph
    PRIVATE ${IncludePath}
    PRIVATE ${HeadersPath}
    PRIVATE ${HeadersPath}/DrawRecordable
    PRIVATE ${HeadersPath}/Window
    
    PUBLIC ${IncludePath}
    PUBLIC $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:VulkanWrapper,INTERFACE_INCLUDE_DIRECTORIES>
    
    PUBLIC ${Vulkan_INCLUDE_DIRS}
    PUBLIC ${glm_INCLUDE_DIRS}
    PRIVATE ${glfw_INCLUDE_DIRS}
    PUBLIC ${vulkan-memory-allocator_INCLUDE_DIRS}
    PUBLIC ${fmt_INCLUDE_DIRS}
    PUBLIC ${spdlog_INCLUDE_DIRS}
    PRIVATE ${stb_INCLUDE_DIRS}
    PRIVATE ${glslang_INCLUDE_DIRS}
    PRIVATE ${spirv-cross_INCLUDE_DIRS}
    PRIVATE ${msdfgen_INCLUDE_DIRS}
)

target_link_libraries (RenderGraph
    PUBLIC ${Vulkan_LIBRARIES}
    PUBLIC ${glm_LIBRARIES}
    PRIVATE ${glfw_LIBRARIES}
    PUBLIC ${vulkan-memory-allocator_LIBRARIES}
    PUBLIC ${fmt_LIBRARIES}
    PUBLIC ${spdlog_LIBRARIES}
    PRIVATE ${stb_LIBRARIES}
    PRIVATE ${glslang_LIBRARIES}
    PRIVATE ${spirv-cross_LIBRARIES}
    PRIVATE ${msdfgen_LIBRARIES}
    PUBLIC Utils
    PUBLIC VulkanWrapper
)
