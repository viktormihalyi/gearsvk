cmake_minimum_required (VERSION 3.15)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

project (VulkanTest)

add_definitions (-D_CRT_SECURE_NO_WARNINGS)


set (TARGET_NAME Tutorial)

add_executable (${TARGET_NAME}
    src/Utils/Assert.cpp
    src/Utils/ImageLoader.cpp
    src/Utils/Logger.cpp
    src/Utils/MessageBox.cpp
    src/Utils/SourceLocation.cpp
    src/Utils/Utils.cpp
    src/VulkanWrapper/ShaderModule.cpp
    src/main.cpp
    src/GLFWWindowProvider.cpp
    src/ShaderReflection.cpp
)

find_package(Vulkan)

target_include_directories (${TARGET_NAME} PRIVATE
    "C:\\VulkanSDK\\1.1.130.0\\Include"
    "C:\\SDL2-2.0.10\\include"

    external/glfw/include
    external/json/single_include
    external/glm
    external/VulkanMemoryAllocator/src
)

target_link_directories (${TARGET_NAME} PRIVATE
    "C:\\VulkanSDK\\1.1.130.0\\Lib"
    "C:\\SDL2-2.0.10\\lib\\x64"
)

target_compile_definitions (${TARGET_NAME} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

target_include_directories (${TARGET_NAME} PRIVATE
    src
    src/VulkanWrapper
    src/Utils
)

target_link_libraries (${TARGET_NAME}
    vulkan-1
    SDL2
)

set (SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set (SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)

set (SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set (SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set (SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set (SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE)

set (SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)

set (BUILD_GMOCK OFF CACHE BOOL "" FORCE)

set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# external libs
add_subdirectory (external/googletest)

add_subdirectory (external/SPIRV-Headers)
add_subdirectory (external/SPIRV-Tools)
add_subdirectory (external/SPIRV-Cross)
add_subdirectory (external/glslang)
add_subdirectory (external/shaderc)
add_subdirectory (external/glfw)


target_include_directories (${TARGET_NAME} PRIVATE
    external/shaderc/libshaderc/include
    external/SPIRV-Cross
)

target_link_libraries (${TARGET_NAME}
    glfw
    shaderc
    spirv-cross-core
    spirv-cross-glsl
    spirv-cross-reflect
)