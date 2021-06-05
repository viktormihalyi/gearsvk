cmake_minimum_required (VERSION 3.20)
project (GearsVk)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (GearsVk_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set (GearsVk_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set (GearsVk_VERSION ${GearsVk_VERSION_MAJOR}.${GearsVk_VERSION_MINOR} CACHE STRING "version" FORCE)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

execute_process (COMMAND
    conan install ${CMAKE_SOURCE_DIR}/conanfile.txt
        --install-folder=${CMAKE_BINARY_DIR}
        --build=missing
        --settings build_type=${CMAKE_BUILD_TYPE})

include (${CMAKE_BINARY_DIR}/conan_paths.cmake)

if (MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4267 /wd4275")
endif ()

add_subdirectory (src/Utils)
add_subdirectory (src/GearsVk)
add_subdirectory (src/GearsPYD)
add_subdirectory (src/Sequence)
add_subdirectory (src/UserInterface)
add_subdirectory (src/Tests)
add_subdirectory (src/SequenceRunner)
