set (BUILD_GMOCK OFF CACHE BOOL "" FORCE)

set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory (external/googletest)
add_subdirectory (external/glslang)
add_subdirectory (external/glfw)
add_subdirectory (external/pybind11)
