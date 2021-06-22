
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/GearsVk)
set (Headers
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
    ${HeadersPath}/RenderGraph/RenderGraphPass.hpp
    ${HeadersPath}/RenderGraph/Resource.hpp
    ${HeadersPath}/RenderGraph/ShaderPipeline.hpp
    ${HeadersPath}/RenderGraph/UniformReflection.hpp
    
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

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/RenderGraph/GraphRenderer.cpp
    ${SourcesPath}/RenderGraph/GraphSettings.cpp
    ${SourcesPath}/RenderGraph/Operation.cpp
    ${SourcesPath}/RenderGraph/RenderGraph.cpp
    ${SourcesPath}/RenderGraph/RenderGraphPass.cpp
    ${SourcesPath}/RenderGraph/Resource.cpp
    ${SourcesPath}/RenderGraph/ShaderPipeline.cpp
    ${SourcesPath}/RenderGraph/UniformReflection.cpp

    ${SourcesPath}/Window/GLFWWindow.cpp
    ${SourcesPath}/Window/SDLWindow.cpp

    #${SourcesPath}/Font.cpp
    ${SourcesPath}/SpdlogInitializer.cpp
    ${SourcesPath}/UniformView.cpp
    ${SourcesPath}/VulkanEnvironment.cpp
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

target_compile_definitions (RenderGraph
    PUBLIC SPDLOG_FMT_EXTERNAL
)

target_precompile_headers (RenderGraph PRIVATE ${SourcesPath}/PCH.hpp)

target_include_directories (RenderGraph
    PRIVATE ${IncludePath}
    PRIVATE ${HeadersPath}
    PRIVATE ${HeadersPath}/Camera
    PRIVATE ${HeadersPath}/RenderGraph
    PRIVATE ${HeadersPath}/RenderGraph/DrawRecordable
    PRIVATE ${HeadersPath}/VulkanWrapper
    PRIVATE ${HeadersPath}/VulkanWrapper/Utils
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
    PUBLIC Utils
    PUBLIC VulkanWrapper
)
