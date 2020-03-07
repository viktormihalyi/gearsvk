cmake_minimum_required (VERSION 3.15)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

project (GearsVk)

find_package (Vulkan)

if (${Vulkan_FOUND})
    message ("Vulkan lib found at "${Vulkan_LIBRARIES})
    message ("Vulkan headers found at "${Vulkan_INCLUDE_DIRS})
else ()
    error ()
endif ()

add_definitions (-D_CRT_SECURE_NO_WARNINGS)

set (TARGET_NAME GearsVk)
set (TARGET_TEST_NAME GearsVkTest)

include (
    cmake/external.cmake
    cmake/Utils.cmake
)

set (COMMON_SOURCE_FILES
    src/VulkanWrapper/ShaderModule.cpp
    src/GearsVk/GLFWWindowProvider.cpp
    src/GearsVk/ShaderReflection.cpp
)

add_executable (${TARGET_NAME}
    ${COMMON_SOURCE_FILES}
    src/GearsVk/main.cpp
)

add_executable (${TARGET_TEST_NAME}
    ${COMMON_SOURCE_FILES}
    src/Tests/TestMain.cpp
)

set (EXTRNAL_INCLUDES
    ${Vulkan_INCLUDE_DIRS}

    external/glfw/include
    external/json/single_include
    external/glm
    external/VulkanMemoryAllocator/src
    external/shaderc/libshaderc/include
    external/SPIRV-Cross
)

set (EXTRNAL_LINKS
    "C:\\SDL2-2.0.10\\lib\\x64"
)

set (COMMON_INCLUDES
    src
    src/GearsVk
    src/VulkanWrapper
    src/Utils
)

set (COMMON_LINKS
    ${Vulkan_LIBRARIES}
    GearsVkUtils
    SDL2
    glfw
    shaderc
    spirv-cross-core
    spirv-cross-glsl
    spirv-cross-reflect
)

set (COMMON_COMPILER_DEFINES
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
