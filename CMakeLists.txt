cmake_minimum_required (VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

message ("cmake binary dir is " ${CMAKE_BINARY_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CONAN_DISABLE_CHECK_COMPILER ON CACHE BOOL "" FORCE)

set (PROJECT_NAME GearsVk)

set (TARGET_UTILS GVkUtils)
set (TARGET_LIB GearsVk)
set (TARGET_VIZHF VizHF)
set (TARGET_VIZHF2 VizHF2)
set (TARGET_TESTS GearsVkTest)
set (TARGET_DLL Gears)

project (${PROJECT_NAME})

find_package (Vulkan REQUIRED)
find_package (Python3 REQUIRED COMPONENTS Interpreter Development)

if (DEFINED CONAN_BUILDINFO_FILE)
    include (${CONAN_BUILDINFO_FILE})
else ()
    include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
endif ()

conan_basic_setup ()

if (${Vulkan_FOUND})
    message ("Vulkan lib found at " ${Vulkan_LIBRARIES})
    message ("Vulkan headers found at " ${Vulkan_INCLUDE_DIRS})
else ()
    error ()
endif ()

if (${Python3_FOUND})
    message ("Python3 lib found at " ${Python3_LIBRARIES})
    message ("Python3 headers found at " ${Python3_INCLUDE_DIRS})
else ()
    error ()
endif ()

if (MSVC)
    set (COMPILER_FLAGS)
else ()
    set (COMPILER_FLAGS -fPIC)
endif ()

add_definitions (-D_CRT_SECURE_NO_WARNINGS)


# ======================  EXTERNAL LIBS ======================
set (ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set (MSDFGEN_BUILD_MSDFGEN_STANDALONE OFF CACHE BOOL "" FORCE)

add_subdirectory (external/msdfgen)
# ============================================================

set (GVK_UTILS_PUBLIC_HEADERS
    Assert.hpp
)

set (GEARSVK_INCLUDE_FOLDERS
    src/GearsVk
    src/GearsVk/RenderGraph
    src/GearsVk/RenderGraph/DrawRecordable
    src/GearsVk/Window
    src/GearsVk/Camera
    src/GearsVk/VulkanWrapper
    src/GearsVk/VulkanWrapper/Utils
    src/Utils
    
    # external libs
    ${Vulkan_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
    
    external/VulkanMemoryAllocator/src
    external/msdfgen
)

add_library (${TARGET_UTILS} SHARED
    src/Utils/Assert.cpp
    src/Utils/Logger.cpp
    src/Utils/MessageBox.cpp
    src/Utils/SourceLocation.cpp
    src/Utils/Utils.cpp
    src/Utils/Time.cpp
    src/Utils/Persistent.cpp
    src/Utils/UUID.cpp
)

add_library (${TARGET_LIB} SHARED
    src/GearsVk/VulkanWrapper/Utils/VulkanUtils.cpp
    src/GearsVk/VulkanWrapper/Utils/BufferTransferable.cpp
    src/GearsVk/VulkanWrapper/Utils/ImageData.cpp
    src/GearsVk/VulkanWrapper/Utils/STBImpl.cpp
    src/GearsVk/VulkanWrapper/DebugUtilsMessenger.cpp
    src/GearsVk/VulkanWrapper/ShaderModule.cpp
    src/GearsVk/VulkanWrapper/Swapchain.cpp
    src/GearsVk/VulkanWrapper/PhysicalDevice.cpp
    src/GearsVk/VulkanWrapper/Pipeline.cpp
    src/GearsVk/VulkanWrapper/ShaderReflection.cpp
    src/GearsVk/VulkanWrapper/Sampler.cpp
    src/GearsVk/VulkanWrapper/DeviceMemory.cpp
    src/GearsVk/VulkanWrapper/Queue.cpp
    src/GearsVk/VulkanWrapper/VulkanObject.cpp
    src/GearsVk/VulkanWrapper/Buffer.cpp
    src/GearsVk/VulkanWrapper/Surface.cpp
    src/GearsVk/VulkanWrapper/Image.cpp
    src/GearsVk/VulkanWrapper/CommandBuffer.cpp

    src/GearsVk/Camera/Frustum.cpp
    src/GearsVk/Camera/Camera.cpp
    
    src/GearsVk/RenderGraph/RenderGraph.cpp
    src/GearsVk/RenderGraph/GraphSettings.cpp
    src/GearsVk/RenderGraph/Resource.cpp
    src/GearsVk/RenderGraph/Operation.cpp
    src/GearsVk/RenderGraph/GraphRenderer.cpp
    src/GearsVk/RenderGraph/ShaderPipeline.cpp
    src/GearsVk/RenderGraph/UniformReflection.cpp
    
    src/GearsVk/Window/GLFWWindow.cpp
    src/GearsVk/Window/SDLWindow.cpp

    src/GearsVk/VulkanEnvironment.cpp
    src/GearsVk/VulkanMemoryAllocatorImpl.cpp
    src/GearsVk/Font.cpp
    src/GearsVk/UniformView.cpp

    external/msdfgen/ext/import-font.cpp
)

add_executable (${TARGET_VIZHF}
    src/VizHF/main.cpp
)

add_executable (${TARGET_VIZHF2}
    src/VizHF2/main.cpp
)

add_executable (${TARGET_TESTS}
    src/Tests/FontRenderingTests.cpp
    src/Tests/RenderGraphTests.cpp
    src/Tests/TestMain.cpp
    src/Tests/GearsTests.cpp
)

add_library (${TARGET_DLL} SHARED
    src/GearsPYD/Gears.cpp
    src/GearsPYD/event/events.cpp
    src/GearsPYD/core/Pass.cpp
    src/GearsPYD/core/PassRenderer.cpp
    src/GearsPYD/core/Stimulus.cpp
    src/GearsPYD/core/StimulusRenderer.cpp
    src/GearsPYD/core/Sequence.cpp
    src/GearsPYD/core/SequenceRenderer.cpp
    src/GearsPYD/core/Response.cpp
    src/GearsPYD/core/Ticker.cpp
    src/GearsPYD/core/PortHandlerWin32.cpp
    src/GearsPYD/core/PortHandlerLinux.cpp
    src/GearsPYD/core/filter/KernelManager.cpp
    src/GearsPYD/core/filter/SpatialFilter.cpp

    src/GearsPYD/gpu/Framebuffer.cpp
    src/GearsPYD/gpu/Nothing.cpp
    src/GearsPYD/gpu/Pointgrid.cpp
    src/GearsPYD/gpu/Quad.cpp
    src/GearsPYD/gpu/RandomSequenceBuffer.cpp
    src/GearsPYD/gpu/Shader.cpp
    src/GearsPYD/gpu/StimulusGrid.cpp
    src/GearsPYD/gpu/Texture.cpp
    src/GearsPYD/gpu/TextureQueue.cpp
    src/GearsPYD/GearsAPIv2.cpp)
    
target_compile_definitions (${TARGET_UTILS} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
    GVK_UTILS_EXPORTS
)   
target_compile_definitions (${TARGET_LIB} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
    GEARSVK_EXPORTS
)
target_compile_definitions (${TARGET_VIZHF} PRIVATE)
target_compile_definitions (${TARGET_VIZHF2} PRIVATE)
target_compile_definitions (${TARGET_TESTS} PRIVATE)
target_compile_definitions (${TARGET_DLL} PRIVATE)

target_include_directories (${TARGET_UTILS} PRIVATE src/Utils)
target_include_directories (${TARGET_LIB} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_VIZHF} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_VIZHF2} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_TESTS} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_DLL} PRIVATE
    ${GEARSVK_INCLUDE_FOLDERS}
    src/GearsPYD
)


target_link_libraries (${TARGET_UTILS})
target_link_libraries (${TARGET_LIB} ${TARGET_UTILS} ${Vulkan_LIBRARIES} ${CONAN_LIBS} msdfgen)
target_link_libraries (${TARGET_VIZHF} ${TARGET_LIB})
target_link_libraries (${TARGET_VIZHF2} ${TARGET_LIB})
target_link_libraries (${TARGET_TESTS} ${TARGET_LIB})
target_link_libraries (${TARGET_DLL} ${TARGET_LIB} ${Python3_LIBRARIES})

target_compile_options (${TARGET_UTILS} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_LIB} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_VIZHF} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_VIZHF2} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_TESTS} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_DLL} PRIVATE ${COMPILER_FLAGS})
