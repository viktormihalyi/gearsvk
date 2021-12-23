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
    Include/RenderGraph/RenderGraphExport.hpp
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
find_package (glslang REQUIRED)
find_package (spirv-cross REQUIRED)
find_package (msdfgen REQUIRED)

target_compile_definitions (RenderGraph
    PUBLIC GLM_FORCE_RADIANS
    PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE
)

target_precompile_headers (RenderGraph PRIVATE PrecompiledHeader.hpp)

set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare (
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        bf71a834948186f4097caa076cd2663c69a10e1e # 0.9.9.8
)

FetchContent_Declare (
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        7d5a16ce714f0b5f4efa3262de22e4d948851525 # 3.3.6
)

FetchContent_MakeAvailable (glm glfw)

target_include_directories (RenderGraph
    PRIVATE
        Include/RenderGraph
        Include/RenderGraph/Drawable
        Include/RenderGraph/Window
        
        $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
        ${glslang_INCLUDE_DIRS}
        ${spirv-cross_INCLUDE_DIRS}
        ${msdfgen_INCLUDE_DIRS}
    
    PUBLIC
        Include
        
        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:VulkanWrapper,INTERFACE_INCLUDE_DIRECTORIES>
        
        ${Vulkan_INCLUDE_DIRS}
        $<TARGET_PROPERTY:glm,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (RenderGraph
    PRIVATE
        glfw
        ${glslang_LIBRARIES}
        ${spirv-cross_LIBRARIES}
        ${msdfgen_LIBRARIES}

    PUBLIC
        Utils
        VulkanWrapper
        ${Vulkan_LIBRARIES}
)
