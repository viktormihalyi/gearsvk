cmake_minimum_required (VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set (PROJECT_NAME GearsVk)
set (TARGET_NAME ${PROJECT_NAME})
set (TARGET_TEST_NAME ${PROJECT_NAME}Test)
set (TARGET_DLL_NAME Gears)

project (GearsVk)

find_package (Vulkan REQUIRED)
find_package (Python3 REQUIRED COMPONENTS Interpreter Development)

if (${Vulkan_FOUND})
    message ("Vulkan lib found at " ${Vulkan_LIBRARIES})
    message ("Vulkan headers found at " ${Vulkan_INCLUDE_DIRS})
else ()
    error ()
endif ()

if (${Python3_FOUND})
    message ("Python3 lib found at " ${Python3_LIBRARIES_RELEASE})
    message ("Python3 headers found at " ${Python3_INCLUDE_DIRS})
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
    src/VulkanWrapper/Utils/VulkanUtils.cpp
    src/VulkanWrapper/Utils/BufferTransferable.cpp
    src/VulkanWrapper/DebugUtilsMessenger.cpp
    src/VulkanWrapper/ShaderModule.cpp
    src/VulkanWrapper/Swapchain.cpp
    src/VulkanWrapper/PhysicalDevice.cpp
    src/GearsVk/Camera/Frustum.cpp
    src/GearsVk/Camera/Camera.cpp
    src/GearsVk/RenderGraph/RenderGraph.cpp
    src/GearsVk/RenderGraph/Resource.cpp
    src/GearsVk/RenderGraph/Operation.cpp
    src/GearsVk/RenderGraph/DrawRecordable/DrawRecordable.cpp
    src/GearsVk/RenderGraph/GraphRenderer.cpp
    src/GearsVk/Window/GLFWWindow.cpp
    src/GearsVk/Window/SDLWindow.cpp
    src/GearsVk/ShaderReflection.cpp
    src/GearsVk/VulkanEnvironment.cpp)

add_executable (${TARGET_NAME}
    ${COMMON_SOURCE_FILES}
    src/GearsVk/main.cpp
)

add_executable (${TARGET_TEST_NAME}
    ${COMMON_SOURCE_FILES}
    src/Tests/TestMain.cpp
)

add_library (${TARGET_DLL_NAME} SHARED
    ${COMMON_SOURCE_FILES}
	
    src/GearsDLL/Gears.cpp
    src/GearsDLL/event/events.cpp
    src/GearsDLL/core/Pass.cpp
    src/GearsDLL/core/PassRenderer.cpp
    src/GearsDLL/core/Stimulus.cpp
    src/GearsDLL/core/StimulusRenderer.cpp
    src/GearsDLL/core/Sequence.cpp
    src/GearsDLL/core/SequenceRenderer.cpp
    src/GearsDLL/core/Response.cpp
    src/GearsDLL/core/Ticker.cpp
    src/GearsDLL/core/PortHandlerWin32.cpp
    src/GearsDLL/core/PortHandlerLinux.cpp
    src/GearsDLL/core/filter/KernelManager.cpp
    src/GearsDLL/core/filter/SpatialFilter.cpp

    src/GearsDLL/gpu/Framebuffer.cpp
    src/GearsDLL/gpu/Nothing.cpp
    src/GearsDLL/gpu/Pointgrid.cpp
    src/GearsDLL/gpu/Quad.cpp
    src/GearsDLL/gpu/RandomSequenceBuffer.cpp
    src/GearsDLL/gpu/Shader.cpp
    src/GearsDLL/gpu/StimulusGrid.cpp
    src/GearsDLL/gpu/Texture.cpp
    src/GearsDLL/gpu/TextureQueue.cpp
    src/GearsDLL/GearsAPIv2.cpp)

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
    ${Python3_INCLUDE_DIRS}
    external/googletest/googletest/include
    external/glfw/include
    external/json/single_include
    external/glm
    external/VulkanMemoryAllocator/src
    external/stb
    external/pybind11/include
    external/glslang
)

set (COMMON_INCLUDES
    src
    src/GearsVk
    src/GearsVk/RenderGraph
    src/GearsVk/RenderGraph/DrawRecordable
    src/GearsVk/Window
    src/GearsVk/Camera
    src/VulkanWrapper
    src/VulkanWrapper/Utils
    src/Utils
)

set (COMMON_LINKS
    ${Vulkan_LIBRARIES}
    glfw
    SPIRV
)

set (COMMON_COMPILER_DEFINES
)

target_include_directories (${TARGET_NAME} PRIVATE ${EXTRNAL_INCLUDES})
target_include_directories (${TARGET_TEST_NAME} PRIVATE ${EXTRNAL_INCLUDES})
target_include_directories (${TARGET_DLL_NAME} PRIVATE ${EXTRNAL_INCLUDES})

target_compile_definitions (${TARGET_NAME} PRIVATE ${COMMON_COMPILER_DEFINES})
target_compile_definitions (${TARGET_TEST_NAME} PRIVATE ${COMMON_COMPILER_DEFINES})
target_compile_definitions (${TARGET_DLL_NAME} PRIVATE ${COMMON_COMPILER_DEFINES})

target_include_directories (${TARGET_NAME} PRIVATE ${COMMON_INCLUDES})
target_include_directories (${TARGET_TEST_NAME} PRIVATE ${COMMON_INCLUDES})
target_include_directories (${TARGET_DLL_NAME} PRIVATE ${COMMON_INCLUDES}
    src/GearsDLL
)

target_link_libraries (${TARGET_NAME} ${COMMON_LINKS})
target_link_libraries (${TARGET_TEST_NAME} ${COMMON_LINKS} gtest)
target_link_libraries (${TARGET_DLL_NAME} ${COMMON_LINKS} ${Python3_LIBRARIES_RELEASE})

if (MSVC)
  set (GEARS_MSVC_FLAGS /MDd)
  target_compile_options(${TARGET_NAME} PRIVATE ${GEARS_MSVC_FLAGS})
  target_compile_options(${TARGET_TEST_NAME} PRIVATE ${GEARS_MSVC_FLAGS})
  target_compile_options(${TARGET_DLL_NAME} PRIVATE ${GEARS_MSVC_FLAGS})
else ()
  set (GEARS_GCC_FLAGS -Wall -Wextra -pedantic)
  target_compile_options(${TARGET_NAME} PRIVATE ${GEARS_GCC_FLAGS})
  target_compile_options(${TARGET_TEST_NAME} PRIVATE ${GEARS_GCC_FLAGS})
  target_compile_options(${TARGET_DLL_NAME} PRIVATE ${GEARS_GCC_FLAGS})
endif ()