cmake_minimum_required (VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

message ("CMAKE_CXX_STANDARD = " ${CMAKE_CXX_STANDARD})
message ("CMAKE_SOURCE_DIR   = " ${CMAKE_SOURCE_DIR})
message ("CMAKE_BINARY_DIR   = " ${CMAKE_BINARY_DIR})
message ("CMAKE_BUILD_TYPE   = " ${CMAKE_BUILD_TYPE})

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CONAN_DISABLE_CHECK_COMPILER ON CACHE BOOL "" FORCE)

set (PROJECT_NAME GearsVk)

set (TARGET_GVKUTILS GVKUtils)
set (TARGET_GVKRENDERER GVKRenderer)
set (TARGET_TEST GVKTest)
set (TARGET_DLL GearsModule)
set (TARGET_DLL_EMBEDDED GearsModuleEmbedded)

project (${PROJECT_NAME})

execute_process (COMMAND
    git submodule update --init --recursive)

execute_process (COMMAND
    conan install ${CMAKE_SOURCE_DIR}/conanfile.txt
        --install-folder=${CMAKE_BINARY_DIR}
        --build=missing
        --settings build_type=${CMAKE_BUILD_TYPE})

include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup ()

find_package (Vulkan REQUIRED)
find_package (Python3 REQUIRED COMPONENTS Interpreter Development)

if (${Vulkan_FOUND})
    message ("Vulkan_LIBRARIES     = " ${Vulkan_LIBRARIES})
    message ("Vulkan_INCLUDE_DIRS  = " ${Vulkan_INCLUDE_DIRS})
else ()
    error ()
endif ()

if (${Python3_FOUND})
    message ("Python3_LIBRARIES    = " ${Python3_LIBRARIES})
    message ("Python3_INCLUDE_DIRS = " ${Python3_INCLUDE_DIRS})
    message ("Python3_EXECUTABLE   = " ${Python3_EXECUTABLE})
else ()
    error ()
endif ()

set (PYTHON_VIRTUALENV_PATH env)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${PYTHON_VIRTUALENV_PATH})
    message ("Creating Python3 virtual environment at ${CMAKE_SOURCE_DIR}/${PYTHON_VIRTUALENV_PATH}")
    execute_process (COMMAND
        ${Python3_EXECUTABLE} -m venv ${CMAKE_SOURCE_DIR}/${PYTHON_VIRTUALENV_PATH})

    if (WIN32)
        execute_process (COMMAND
            ${CMAKE_SOURCE_DIR}/${PYTHON_VIRTUALENV_PATH}/Scripts/activate.bat)
    else ()
        execute_process (COMMAND
            source ${CMAKE_SOURCE_DIR}/${PYTHON_VIRTUALENV_PATH}/bin/activate)
    endif ()

    message ("Insatlling pip dependencies.")
    execute_process (COMMAND
        ${Python3_EXECUTABLE} -m pip install -r ${CMAKE_SOURCE_DIR}/requirements.txt)
else ()
    message ("Python3 virtual environment already exists at ${CMAKE_SOURCE_DIR}/${PYTHON_VIRTUALENV_PATH}")
endif ()


if (MSVC)
    set (COMPILER_FLAGS /wd4251 /wd4244 /wd4267 /wd4275 /wd4099)
else ()
    set (COMPILER_FLAGS -fPIC)
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
    src/Utils
    src/Tests
    src/GearsPYD
    
    # external libs
    ${Vulkan_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
    external/msdfgen
)

add_library (${TARGET_GVKUTILS} SHARED
    src/Utils/Assert.cpp
    src/Utils/MessageBox.cpp
    src/Utils/SourceLocation.cpp
    src/Utils/Utils.cpp
    src/Utils/Time.cpp
    src/Utils/UUID.cpp
)

add_library (${TARGET_GVKRENDERER} SHARED
    src/GearsVk/VulkanWrapper/Utils/VulkanUtils.cpp
    src/GearsVk/VulkanWrapper/Utils/BufferTransferable.cpp
    src/GearsVk/VulkanWrapper/Utils/ImageData.cpp
    src/GearsVk/VulkanWrapper/Utils/STBImpl.cpp
    src/GearsVk/VulkanWrapper/DebugUtilsMessenger.cpp
    src/GearsVk/VulkanWrapper/ShaderModule.cpp
    src/GearsVk/VulkanWrapper/Swapchain.cpp
    src/GearsVk/VulkanWrapper/PhysicalDevice.cpp
    src/GearsVk/VulkanWrapper/Instance.cpp
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

    external/msdfgen/ext/import-font.cpp)

add_executable (${TARGET_TEST}
    src/Tests/GoogleTestEnvironment.cpp
    src/Tests/FontRenderingTests.cpp
    src/Tests/RenderGraphTests.cpp
    src/Tests/TestMain.cpp
    src/Tests/GearsTests.cpp
    src/Tests/VizHFTests.cpp)

set (GEARSMODULE_COMMON_SOURCES
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
    src/GearsPYD/GearsAPIv2.cpp
    src/GearsPYD/StimulusAdapterForPresentable.cpp
    src/GearsPYD/StimulusAdapterView.cpp
    src/GearsPYD/SequenceAdapter.cpp
)

add_library (${TARGET_DLL} SHARED
    ${GEARSMODULE_COMMON_SOURCES}
    src/GearsPYD/GearsModule.cpp
)

add_library (${TARGET_DLL_EMBEDDED} SHARED
    ${GEARSMODULE_COMMON_SOURCES}
    src/GearsPYD/GearsModuleEmbedded.cpp
)
    
target_compile_definitions (${TARGET_GVKUTILS} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_SOURCE_DIR}\"
    GVK_UTILS_EXPORTS
)   
target_compile_definitions (${TARGET_GVKRENDERER} PRIVATE
    GVK_RENDERER_EXPORTS
)
target_compile_definitions (${TARGET_TEST} PRIVATE)
target_compile_definitions (${TARGET_DLL} PRIVATE GEARS_EXPORTS)
target_compile_definitions (${TARGET_DLL_EMBEDDED} PRIVATE GEARS_EMBEDDED_EXPORTS)

target_include_directories (${TARGET_GVKUTILS} PRIVATE src/Utils)
target_include_directories (${TARGET_GVKRENDERER} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_TEST} PRIVATE ${GEARSVK_INCLUDE_FOLDERS})
target_include_directories (${TARGET_DLL} PRIVATE
    ${GEARSVK_INCLUDE_FOLDERS}
    src/GearsPYD
)
target_include_directories (${TARGET_DLL_EMBEDDED} PRIVATE
    ${GEARSVK_INCLUDE_FOLDERS}
    src/GearsPYD
)


if (WIN32)
target_link_libraries (${TARGET_GVKUTILS} rpcrt4)
elseif (UNIX)
target_link_libraries (${TARGET_GVKUTILS} uuid)
else ()
target_link_libraries (${TARGET_GVKUTILS})
endif ()

target_link_libraries (${TARGET_GVKRENDERER} ${TARGET_GVKUTILS} ${Vulkan_LIBRARIES} ${CONAN_LIBS} msdfgen)
target_link_libraries (${TARGET_DLL} ${TARGET_GVKRENDERER} ${Python3_LIBRARIES})
target_link_libraries (${TARGET_DLL_EMBEDDED} ${TARGET_GVKRENDERER} ${Python3_LIBRARIES})
target_link_libraries (${TARGET_TEST} ${TARGET_GVKRENDERER} ${TARGET_DLL_EMBEDDED} ${CONAN_LIBS})

target_compile_options (${TARGET_GVKUTILS} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_GVKRENDERER} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_TEST} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_DLL} PRIVATE ${COMPILER_FLAGS})
target_compile_options (${TARGET_DLL_EMBEDDED} PRIVATE ${COMPILER_FLAGS})


set (UI_PATH ${CMAKE_SOURCE_DIR}/src/UserInterface)

add_custom_command (
    TARGET ${TARGET_DLL}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${UI_PATH}
        ${CMAKE_BINARY_DIR}/bin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Copying UserInterface..."
)

if (${CMAKE_BUILD_TYPE} EQUAL "Debug")
    set (PYTHON_MODULE_POSTFIX d)
endif ()

if (WIN32)
    set (PYD_MODULE_EXTENSION_SOURCE dll)
    set (PYD_MODULE_EXTENSION_TARGET pyd)
else ()
    set (PYD_MODULE_EXTENSION_SOURCE so)
    set (PYD_MODULE_EXTENSION_TARGET so)
endif ()

add_custom_command (
    TARGET ${TARGET_DLL}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/bin/${TARGET_DLL}.${PYD_MODULE_EXTENSION_SOURCE}
        ${CMAKE_BINARY_DIR}/bin/${TARGET_DLL}${PYTHON_MODULE_POSTFIX}.${PYD_MODULE_EXTENSION_TARGET}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Copying ${TARGET_DLL}..."
)

add_custom_command (
    TARGET ${TARGET_DLL}
    POST_BUILD
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_BINARY_DIR}/bin/setup.py build -b ${CMAKE_BINARY_DIR}/install
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running setup.py..."
)