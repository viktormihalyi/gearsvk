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
add_subdirectory (external/pybind11)
