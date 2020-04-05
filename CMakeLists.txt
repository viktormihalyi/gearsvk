cmake_minimum_required (VERSION 3.15.1)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set (PROJECT_NAME GearsVk)
set (TARGET_NAME ${PROJECT_NAME})
set (TARGET_TEST_NAME ${PROJECT_NAME}Test)
set (TARGET_DLL_NAME ${PROJECT_NAME}DLL)

project (GearsVk)

set (SDL2_INCLUDE_DIR "C:\\Dev\\SDL2-2.0.12\\include")
set (SDL2_LIB_DIR "C:\\Dev\\SDL2-2.0.12\\lib\\x64")

find_package (Vulkan REQUIRED)
find_package (Python REQUIRED COMPONENTS Interpreter Development)

if (${Vulkan_FOUND})
    message ("Vulkan lib found at "${Vulkan_LIBRARIES})
    message ("Vulkan headers found at "${Vulkan_INCLUDE_DIRS})
else ()
    error ()
endif ()

if (${Python_FOUND})
    message ("Python lib found at "${Python_LIBRARIES})
    message ("Python headers found at "${Python_INCLUDE_DIRS})
else ()
    error ()
endif ()

add_definitions (-D_CRT_SECURE_NO_WARNINGS)

include (
    cmake/external.cmake
)

set (COMMON_SOURCE_FILES
    src/Utils/Assert.cpp
    src/Utils/Logger.cpp
    src/Utils/MessageBox.cpp
    src/Utils/SourceLocation.cpp
    src/Utils/Utils.cpp
    src/VulkanWrapper/VulkanUtils.cpp
    src/VulkanWrapper/DebugUtilsMessenger.cpp
    src/VulkanWrapper/ShaderModule.cpp
    src/GearsVk/RenderGraph/RenderGraph.cpp
    src/GearsVk/RenderGraph/Resource.cpp
    src/GearsVk/RenderGraph/Operation.cpp
    src/GearsVk/Window/GLFWWindow.cpp
    src/GearsVk/Window/SDLWindow.cpp
    src/GearsVk/ShaderReflection.cpp
    src/GearsVk/RenderGraph/DrawRecordable.cpp
    src/VulkanWrapper/BufferTransferable.cpp)

add_executable (${TARGET_NAME}
    ${COMMON_SOURCE_FILES}
    src/GearsVk/main.cpp
)

add_executable (${TARGET_TEST_NAME}
    ${COMMON_SOURCE_FILES}
    src/Tests/TestMain.cpp
)

add_library (${TARGET_DLL_NAME} SHARED
    
    src/Utils/SourceLocation.cpp

    #src/AdaptedGearsSource/Gears.cpp
    
    src/GearsSource/Gears.cpp

    src/GearsSource/event/events.cpp

    src/GearsSource/core/Pass.cpp
    src/GearsSource/core/PassRenderer.cpp
    src/GearsSource/core/Stimulus.cpp
    src/GearsSource/core/StimulusRenderer.cpp
    src/GearsSource/core/Sequence.cpp
    src/GearsSource/core/SequenceRenderer.cpp
    src/GearsSource/core/Response.cpp
    src/GearsSource/core/Ticker.cpp
    src/GearsSource/core/PortHandlerWin32.cpp
    src/GearsSource/core/PortHandlerLinux.cpp
    src/GearsSource/core/filter/KernelManager.cpp
    src/GearsSource/core/filter/SpatialFilter.cpp
    src/GearsSource/core/StimulusWindow.cpp
    src/GearsSource/core/StimulusWindowLinux.cpp
    src/GearsSource/core/StimulusWindowWin32.cpp

    src/GearsSource/gpu/Framebuffer.cpp
    src/GearsSource/gpu/Nothing.cpp
    src/GearsSource/gpu/Pointgrid.cpp
    src/GearsSource/gpu/Quad.cpp
    src/GearsSource/gpu/RandomSequenceBuffer.cpp
    src/GearsSource/gpu/Shader.cpp
    src/GearsSource/gpu/StimulusGrid.cpp
    src/GearsSource/gpu/Texture.cpp
    src/GearsSource/gpu/TextureQueue.cpp

)

target_compile_definitions (${TARGET_NAME} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

target_compile_definitions (${TARGET_TEST_NAME} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

target_compile_definitions (${TARGET_DLL_NAME} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

set (EXTRNAL_INCLUDES
    ${Vulkan_INCLUDE_DIRS}
    ${Python_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    external/googletest/googletest/include
    external/glfw/include
    external/json/single_include
    external/glm
    external/VulkanMemoryAllocator/src
    external/shaderc/libshaderc/include
    external/SPIRV-Cross
    external/stb
    external/pybind11/include
)

set (EXTRNAL_LINKS
    ${SDL2_LIB_DIR}
)

set (COMMON_INCLUDES
    src
    src/GearsVk
    src/GearsVk/RenderGraph
    src/GearsVk/Window
    src/VulkanWrapper
    src/Utils

)

set (COMMON_LINKS
    ${Vulkan_LIBRARIES}
    ${Python_LIBRARIES}
    SDL2
    glfw
    shaderc
    spirv-cross-core
    spirv-cross-glsl
    spirv-cross-reflect
    gtest
)

set (COMMON_COMPILER_DEFINES
)

target_include_directories (${TARGET_NAME} PRIVATE ${EXTRNAL_INCLUDES})
target_include_directories (${TARGET_TEST_NAME} PRIVATE ${EXTRNAL_INCLUDES})
target_include_directories (${TARGET_DLL_NAME} PRIVATE ${EXTRNAL_INCLUDES})

target_link_directories (${TARGET_NAME} PRIVATE ${EXTRNAL_LINKS})
target_link_directories (${TARGET_TEST_NAME} PRIVATE ${EXTRNAL_LINKS})
target_link_directories (${TARGET_DLL_NAME} PRIVATE ${EXTRNAL_LINKS})

target_compile_definitions (${TARGET_NAME} PRIVATE ${COMMON_COMPILER_DEFINES})
target_compile_definitions (${TARGET_TEST_NAME} PRIVATE ${COMMON_COMPILER_DEFINES})
target_compile_definitions (${TARGET_DLL_NAME} PRIVATE ${COMMON_COMPILER_DEFINES})

target_include_directories (${TARGET_NAME} PRIVATE ${COMMON_INCLUDES})
target_include_directories (${TARGET_TEST_NAME} PRIVATE ${COMMON_INCLUDES})
target_include_directories (${TARGET_DLL_NAME} PRIVATE ${COMMON_INCLUDES}
    src/GearsSource
)

target_link_libraries (${TARGET_NAME} ${COMMON_LINKS})
target_link_libraries (${TARGET_TEST_NAME} ${COMMON_LINKS})
target_link_libraries (${TARGET_DLL_NAME} ${COMMON_LINKS})
