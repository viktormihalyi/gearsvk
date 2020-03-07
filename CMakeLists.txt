cmake_minimum_required (VERSION 3.15)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

project (GearsVk)

find_package (Vulkan)

# external libs

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

add_subdirectory (external/googletest)

add_subdirectory (external/SPIRV-Headers)
add_subdirectory (external/SPIRV-Tools)
add_subdirectory (external/SPIRV-Cross)
add_subdirectory (external/glslang)
add_subdirectory (external/shaderc)
add_subdirectory (external/glfw)



add_definitions (-D_CRT_SECURE_NO_WARNINGS)


set (TARGET_NAME GearsVk)
set (TARGET_TEST_NAME GearsVkTest)


set (COMMON_SOURCE_FILES
    src/Utils/Assert.cpp
    src/Utils/ImageLoader.cpp
    src/Utils/Logger.cpp
    src/Utils/MessageBox.cpp
    src/Utils/SourceLocation.cpp
    src/Utils/Utils.cpp
    src/VulkanWrapper/ShaderModule.cpp
    src/GLFWWindowProvider.cpp
    src/ShaderReflection.cpp
)

add_executable (${TARGET_NAME}
    ${COMMON_SOURCE_FILES}
    src/main.cpp
)

add_executable (${TARGET_TEST_NAME}
    ${COMMON_SOURCE_FILES}
    src/tests/TestMain.cpp
)

set (EXTRNAL_INCLUDES
    "C:\\VulkanSDK\\1.1.130.0\\Include"
    "C:\\SDL2-2.0.10\\include"

    external/glfw/include
    external/json/single_include
    external/glm
    external/VulkanMemoryAllocator/src
    external/shaderc/libshaderc/include
    external/SPIRV-Cross
)

set (EXTRNAL_LINKS
    "C:\\VulkanSDK\\1.1.130.0\\Lib"
    "C:\\SDL2-2.0.10\\lib\\x64"
)

set (COMMON_INCLUDES
    src
    src/VulkanWrapper
    src/Utils
)

set (COMMON_LINKS
    vulkan-1
    SDL2
    glfw
    shaderc
    spirv-cross-core
    spirv-cross-glsl
    spirv-cross-reflect
)
set (COMMON_COMPILER_DEFINES
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

target_include_directories (${TARGET_NAME} PRIVATE ${EXTRNAL_INCLUDES})
target_include_directories (${TARGET_TEST_NAME} PRIVATE ${EXTRNAL_INCLUDES})

target_link_directories (${TARGET_NAME} PRIVATE ${EXTRNAL_LINKS})
target_link_directories (${TARGET_TEST_NAME} PRIVATE ${EXTRNAL_LINKS})

target_compile_definitions (${TARGET_NAME} PRIVATE ${COMMON_COMPILER_DEFINES})
target_compile_definitions (${TARGET_TEST_NAME} PRIVATE ${COMMON_COMPILER_DEFINES})

target_include_directories (${TARGET_NAME} PRIVATE ${COMMON_INCLUDES})
target_include_directories (${TARGET_TEST_NAME} PRIVATE ${COMMON_INCLUDES})

target_link_libraries (${TARGET_NAME} ${COMMON_LINKS})
target_link_libraries (${TARGET_TEST_NAME} ${COMMON_LINKS})
