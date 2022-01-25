enable_testing ()

set (Sources
    Sources/TestEnvironment.cpp
    Sources/TestEnvironment.hpp
    Sources/TestMain.cpp
    Sources/GearsTests.cpp
    Sources/LogInitializer.cpp
)

add_executable (GearsTest ${Sources})

target_precompile_headers (GearsTest PRIVATE PrecompiledHeader.hpp)

set (BUILD_GMOCK OFF CACHE BOOL "" FORCE)

FetchContent_Declare (
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
)

FetchContent_MakeAvailable (googletest)

target_include_directories (GearsTest
    PRIVATE
        Sources

        $<TARGET_PROPERTY:GearsModule,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:gtest,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (GearsTest
    PUBLIC
        Sequence
        GearsModule
        RenderGraph

        gtest
)

set_target_properties (GearsTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_test (NAME GearsTest
          COMMAND GearsTest
          WORKING_DIRECTORY bin)

add_custom_target (GearsTest_CopyTestFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TestData
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TestData
)

