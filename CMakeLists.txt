cmake_minimum_required (VERSION 3.15)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

project (VulkanTest)

add_definitions (-D_CRT_SECURE_NO_WARNINGS)

set (TARGET_NAME Tutorial)

add_executable (${TARGET_NAME}
    src/Utils/Assert.cpp
    src/Utils/Logger.cpp
    src/Utils/MessageBox.cpp
    src/Utils/SourceLocation.cpp
    src/Utils/Utils.cpp
    src/main.cpp
    src/GLFWWindowProvider.cpp
)

target_include_directories (${TARGET_NAME} PRIVATE
    "C:\\VulkanSDK\\1.1.130.0\\Include"
    "C:\\glfw-3.3.1.bin.WIN64\\include"
    "C:\\SDL2-2.0.10\\include"
    "${googletest_SOURCE_DIR}/googletest/include"
)

target_link_directories (${TARGET_NAME} PRIVATE
    "C:\\VulkanSDK\\1.1.130.0\\Lib"
    "C:\\glfw-3.3.1.bin.WIN64\\lib-vc2019"
    "C:\\SDL2-2.0.10\\lib\\x64"
)

target_compile_definitions (${TARGET_NAME} PRIVATE
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

target_include_directories (${TARGET_NAME} PRIVATE
    src
    src/Utils
)

target_link_libraries (${TARGET_NAME}
    vulkan-1
    glfw3
    SDL2
)
