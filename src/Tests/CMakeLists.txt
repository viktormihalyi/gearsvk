enable_testing ()

set (Sources
    Sources/Camera.cpp
    Sources/Frustum.cpp
    Sources/TestEnvironment.cpp
    Sources/TestEnvironment.hpp
    Sources/TestMain.cpp

    Sources/LCGTest.cpp
    Sources/RenderGraphPassTest.cpp
    Sources/RenderGraphAbstractionTest.cpp
    Sources/RenderGraphTests.cpp
    Sources/VizHFTests.cpp
    Sources/FontRenderingTests.cpp
    Sources/GearsTests.cpp

    Sources/LogInitializer.cpp
)

add_executable (GVKTest ${Sources})

find_package (GTest REQUIRED)
find_package (stb REQUIRED)

target_precompile_headers (GVKTest PRIVATE PrecompiledHeader.hpp)

target_include_directories (GVKTest
    PRIVATE
        Sources

        ${GTest_INCLUDE_DIRS}
        ${stb_INCLUDE_DIRS}

        $<TARGET_PROPERTY:GearsModule,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (GVKTest
    PUBLIC
        Sequence
        GearsModule
        RenderGraph
        Utils
        ${GTest_LIBRARIES}
)

add_test (NAME GVKTest
          COMMAND GVKTest
          WORKING_DIRECTORY bin)

add_custom_target (GVKTest_CopyTestFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TestData
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TestData
)
