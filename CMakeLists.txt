cmake_minimum_required (VERSION 3.16)

include (FetchContent)
set (FETCHCONTENT_FULLY_DISCONNECTED OFF CACHE BOOL "" FORCE)
set (FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)

project (GearsVk)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (GearsVk_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set (GearsVk_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set (GearsVk_VERSION ${GearsVk_VERSION_MAJOR}.${GearsVk_VERSION_MINOR} CACHE STRING "version" FORCE)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)


if (MSVC)
    if (CMAKE_UNITY_BUILD)
        add_compile_options (/bigobj)   # increase number of sections in .obj file
    endif ()
    add_compile_options (/MP)           # build with multiple processes

    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")           # .pdb was not found with .lib
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")     # .pdb was not found with .lib
    set (CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")     # .pdb was not found with .lib
endif ()

# ========== CONAN ==========

# FindPython takes ages on Windows after including all those conan folders
# this is a workaround by calling it earlier
find_package (Python3 REQUIRED COMPONENTS Interpreter Development)

if (UNIX)
set (WHICH_COMMAND which)
else ()
set (WHICH_COMMAND where)
endif ()

execute_process (COMMAND ${WHICH_COMMAND} conan OUTPUT_VARIABLE ConanPath RESULT_VARIABLE ret)

if (ret EQUAL "0")
message ("Found conan: " ${ConanPath})
else ()
message (FATAL_ERROR "Could not find conan.")
endif ()

execute_process (
    COMMAND conan install ${CMAKE_SOURCE_DIR}/conanfile.txt
                --install-folder=${CMAKE_BINARY_DIR}
                --build=missing
                --settings build_type=${CMAKE_BUILD_TYPE}
)

include (${CMAKE_BINARY_DIR}/conan_paths.cmake)

# ========== MODULES ==========

add_subdirectory (src/RenderGraph)
add_subdirectory (src/RenderGraphTest)
add_subdirectory (src/Sequence)
add_subdirectory (src/GearsPYD)
add_subdirectory (src/GearsTest)
add_subdirectory (src/SequenceRunner)
add_subdirectory (src/UserInterface)
