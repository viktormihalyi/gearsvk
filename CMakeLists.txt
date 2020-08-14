cmake_minimum_required (VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

message ("cmake binary dir is " ${CMAKE_BINARY_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CONAN_DISABLE_CHECK_COMPILER ON CACHE BOOL "" FORCE)

set (PROJECT_NAME GearsVk)

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
    include (build/conanbuildinfo.cmake)
endif ()

conan_basic_setup ()

if (${Vulkan_FOUND})
    message ("Vulkan lib found at " ${Vulkan_LIBRARIES})
    message ("Vulkan headers found at " ${Vulkan_INCLUDE_DIRS})
else ()
    error ()
endif ()

if (${Python3_FOUND})
    message ("Python3 headers found at " ${Python3_INCLUDE_DIRS})
else ()
    error ()
endif ()

if (MSVC)
    set (COMPILER_FLAGS /W4 /wd4251 /wd4100 /wd4189)
else ()
    set (COMPILER_FLAGS -Wall -Wextra -pedantic -fPIC)
endif ()

add_definitions (-D_CRT_SECURE_NO_WARNINGS)


# ======================  EXTERNAL LIBS ======================
set (ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set (MSDFGEN_BUILD_MSDFGEN_STANDALONE OFF CACHE BOOL "" FORCE)

add_subdirectory (external/msdfgen)
# ============================================================


set (GEARSVK_INCLUDE_FOLDERS
    src/GearsVk
    src/GearsVk/RenderGraph
    src/GearsVk/RenderGraph/DrawRecordable
    src/GearsVk/Window
    src/GearsVk/Camera
    src/GearsVk/VulkanWrapper
    src/GearsVk/VulkanWrapper/Utils
    src/GearsVk/Utils
    
    # external libs
    ${Vulkan_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
    
    external/VulkanMemoryAllocator/src
    external/msdfgen
)

add_library (${TARGET_LIB} SHARED
    src/GearsVk/Utils/Assert.cpp
    src/GearsVk/Utils/Logger.cpp
    src/GearsVk/Utils/MessageBox.cpp
    src/GearsVk/Utils/SourceLocation.cpp
    src/GearsVk/Utils/Utils.cpp
    src/GearsVk/Utils/Time.cpp
    src/GearsVk/Utils/Persistent.cpp
    src/GearsVk/Utils/UUID.cpp
    
    src/GearsVk/VulkanWrapper/Utils/VulkanUtils.cpp
    src/GearsVk/VulkanWrapper/Utils/BufferTransferable.cpp
    src/GearsVk/VulkanWrapper/DebugUtilsMessenger.cpp
    src/GearsVk/VulkanWrapper/ShaderModule.cpp
    src/GearsVk/VulkanWrapper/Swapchain.cpp
    src/GearsVk/VulkanWrapper/PhysicalDevice.cpp
    src/GearsVk/VulkanWrapper/Pipeline.cpp
    src/GearsVk/RenderGraph/ShaderPipeline.cpp
    src/GearsVk/VulkanWrapper/ShaderReflection.cpp

    src/GearsVk/Camera/Frustum.cpp
    src/GearsVk/Camera/Camera.cpp
    
    src/GearsVk/RenderGraph/RenderGraph.cpp
    src/GearsVk/RenderGraph/Resource.cpp
    src/GearsVk/RenderGraph/Operation.cpp
    src/GearsVk/RenderGraph/GraphRenderer.cpp
    
    src/GearsVk/Window/GLFWWindow.cpp
    src/GearsVk/Window/SDLWindow.cpp

    src/GearsVk/VulkanEnvironment.cpp
    src/GearsVk/VulkanMemoryAllocatorImpl.cpp
    
    src/GearsVk/SDF.cpp
    external/msdfgen/ext/import-font.cpp # THIS FUCK DOESNT LINK FREETYPED FOR DEBUG BUILDS
)

add_executable (${TARGET_VIZHF}
    src/VizHF/main.cpp
)

add_executable (${TARGET_VIZHF2}
    src/VizHF2/main.cpp
)

add_executable (${TARGET_TESTS}
    src/Tests/RenderGraphTests.cpp
    src/Tests/TestMain.cpp
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
    
target_compile_definitions (${TARGET_LIB} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
    GEARSVK_EXPORTS
)
target_compile_definitions (${TARGET_VIZHF} PRIVATE)
target_compile_definitions (${TARGET_VIZHF2} PRIVATE)
target_compile_definitions (${TARGET_TESTS} PRIVATE)
target_compile_definitions (${TARGET_DLL} PRIVATE)


target_include_directories (${TARGET_LIB} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_VIZHF} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_VIZHF2} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_TESTS} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_DLL} PRIVATE
    ${GEARSVK_INCLUDE_FOLDERS}
    src/GearsPYD
)


target_link_libraries (${TARGET_LIB} ${Vulkan_LIBRARIES} ${CONAN_LIBS} msdfgen)
target_link_libraries (${TARGET_VIZHF} ${TARGET_LIB})
target_link_libraries (${TARGET_VIZHF2} ${TARGET_LIB})
target_link_libraries (${TARGET_TESTS} ${TARGET_LIB})
target_link_libraries (${TARGET_DLL} ${TARGET_LIB})

target_compile_options (${TARGET_LIB} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_VIZHF} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_VIZHF2} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_TESTS} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_DLL} PRIVATE ${COMPILER_FLAGS})
