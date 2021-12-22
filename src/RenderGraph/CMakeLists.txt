add_library (RenderGraph SHARED)

set (Headers
    Include/RenderGraph/Drawable/Drawable.hpp
    Include/RenderGraph/Drawable/DrawableInfo.hpp
    Include/RenderGraph/Drawable/FullscreenQuad.hpp

    Include/RenderGraph/GraphRenderer.hpp
    Include/RenderGraph/GraphSettings.hpp
    Include/RenderGraph/DescriptorBindable.hpp
    Include/RenderGraph/Node.hpp
    Include/RenderGraph/Operation.hpp
    Include/RenderGraph/RenderGraph.hpp
    Include/RenderGraph/RenderGraphPass.hpp
    Include/RenderGraph/Resource.hpp
    Include/RenderGraph/ShaderPipeline.hpp
    Include/RenderGraph/ComputeShaderPipeline.hpp
    Include/RenderGraph/UniformReflection.hpp
    
    Include/RenderGraph/Window/GLFWWindow.hpp
    Include/RenderGraph/Window/SDLWindow.hpp
    Include/RenderGraph/Window/Window.hpp

    Include/RenderGraph/Font.hpp
    Include/RenderGraph/RenderGraphAPI.hpp
    Include/RenderGraph/BufferView.hpp
    Include/RenderGraph/VulkanEnvironment.hpp

    Include/RenderGraph/ShaderReflectionToVertexAttribute.hpp
    Include/RenderGraph/ShaderReflectionToDescriptor.hpp
    Include/RenderGraph/ShaderReflectionToAttachment.hpp
)

set (Sources
    Sources/Drawable/Drawable.cpp
    Sources/Drawable/DrawableInfo.cpp

    Sources/GraphRenderer.cpp
    Sources/GraphSettings.cpp
    Sources/Operation.cpp
    Sources/RenderGraph.cpp
    Sources/RenderGraphPass.cpp
    Sources/Resource.cpp
    Sources/ShaderPipeline.cpp
    Sources/ComputeShaderPipeline.cpp
    Sources/UniformReflection.cpp

    Sources/Window/GLFWWindow.cpp
    Sources/Window/SDLWindow.cpp

    Sources/Font.cpp
    Sources/LogInitializer.cpp
    Sources/BufferView.cpp
    Sources/VulkanEnvironment.cpp
    
    Sources/ShaderReflectionToVertexAttribute.cpp
    Sources/ShaderReflectionToDescriptor.cpp
    Sources/ShaderReflectionToAttachment.cpp
)

target_sources (RenderGraph PRIVATE ${Headers} ${Sources})

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

target_precompile_headers (RenderGraph PRIVATE PrecompiledHeader.hpp)

target_include_directories (RenderGraph
    PRIVATE
        Include/RenderGraph
        Include/RenderGraph/Drawable
        Include/RenderGraph/Window
        
        ${glfw_INCLUDE_DIRS}
        ${stb_INCLUDE_DIRS}
        ${glslang_INCLUDE_DIRS}
        ${spirv-cross_INCLUDE_DIRS}
        ${msdfgen_INCLUDE_DIRS}
    
    PUBLIC
        Include
        
        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:VulkanWrapper,INTERFACE_INCLUDE_DIRECTORIES>
        
        ${Vulkan_INCLUDE_DIRS}
        ${glm_INCLUDE_DIRS}
        ${vulkan-memory-allocator_INCLUDE_DIRS}
        ${fmt_INCLUDE_DIRS}
        ${spdlog_INCLUDE_DIRS}
)

target_link_libraries (RenderGraph
    PRIVATE
        ${glfw_LIBRARIES}
        ${stb_LIBRARIES}
        ${glslang_LIBRARIES}
        ${spirv-cross_LIBRARIES}
        ${msdfgen_LIBRARIES}

    PUBLIC
        Utils
        VulkanWrapper
        ${Vulkan_LIBRARIES}
        ${glm_LIBRARIES}
        ${vulkan-memory-allocator_LIBRARIES}
        ${fmt_LIBRARIES}
        ${spdlog_LIBRARIES}
)
