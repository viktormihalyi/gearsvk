cmake_minimum_required (VERSION 3.16)
project (GearsVk)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (GearsVk_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set (GearsVk_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set (GearsVk_VERSION ${GearsVk_VERSION_MAJOR}.${GearsVk_VERSION_MINOR} CACHE STRING "version" FORCE)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ========== CONAN ==========

# Only run conan install when conanfile.txt changes

execute_process (
    COMMAND ${CMAKE_COMMAND} -E sha512sum ${CMAKE_SOURCE_DIR}/conanfile.txt
    OUTPUT_VARIABLE ConanfileChecksumCurrent
    OUTPUT_STRIP_TRAILING_WHITESPACE)
    
set (CONANFILE_CHECKSUM_LAST_PATH ${CMAKE_BINARY_DIR}/ConanfileChecksumLast.txt)
if (EXISTS ${CONANFILE_CHECKSUM_LAST_PATH})
    file (STRINGS ${CONANFILE_CHECKSUM_LAST_PATH} ConanfileChecksumLast)
endif ()

if (NOT "${ConanfileChecksumCurrent}" STREQUAL "${ConanfileChecksumLast}")
    message (NOTICE "[CONANFILE_CHECKSUM] Current: ${ConanfileChecksumCurrent}")
    message (NOTICE "[CONANFILE_CHECKSUM] Last:    ${ConanfileChecksumLast}")
    message (NOTICE "Running conan install...")
    execute_process (
        COMMAND conan install ${CMAKE_SOURCE_DIR}/conanfile.txt
                    --install-folder=${CMAKE_BINARY_DIR}
                    --build=missing
                    --settings build_type=${CMAKE_BUILD_TYPE}
        OUTPUT_VARIABLE ret)
    if (ret EQUAL "1")
        file (WRITE ${CMAKE_BINARY_DIR}/ConanfileChecksumLast.txt "${ConanfileChecksumCurrent}")
    else ()
        message (NOTICE "Conan install failed.")
    endif ()
else ()
    message (NOTICE "NOT Running conan install.")
endif ()

include (${CMAKE_BINARY_DIR}/conan_paths.cmake)

# ========== MODULES ==========

add_subdirectory (src/Utils)
add_subdirectory (src/VulkanWrapper)
add_subdirectory (src/RenderGraph)
add_subdirectory (src/Sequence)
add_subdirectory (src/PySequence)
add_subdirectory (src/GearsPYD)
add_subdirectory (src/Tests)
add_subdirectory (src/SequenceRunner)
add_subdirectory (src/UserInterface)
