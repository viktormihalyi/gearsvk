enable_testing ()

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/Camera.cpp
    ${SourcesPath}/Frustum.cpp
    ${SourcesPath}/TestEnvironment.cpp
    ${SourcesPath}/TestEnvironment.hpp
    ${SourcesPath}/TestMain.cpp

    ${SourcesPath}/LCGTest.cpp
    ${SourcesPath}/RenderGraphPassTest.cpp
    ${SourcesPath}/RenderGraphAbstractionTest.cpp
    ${SourcesPath}/RenderGraphTests.cpp
    ${SourcesPath}/VizHFTests.cpp
    ${SourcesPath}/XorShiftTest.cpp
    ${SourcesPath}/FontRenderingTests.cpp
    ${SourcesPath}/GearsTests.cpp
)

add_executable (GVKTest ${Sources})

find_package (GTest REQUIRED)
find_package (stb REQUIRED)

target_include_directories (GVKTest
    PRIVATE ${SourcesPath}

    PRIVATE ${GTest_INCLUDE_DIRS}
    PRIVATE ${stb_INCLUDE_DIRS}

    PRIVATE $<TARGET_PROPERTY:GearsModule,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>

)

target_link_libraries (GVKTest
    PUBLIC ${GTest_LIBRARIES}
    PRIVATE Sequence
    PRIVATE GearsModule
    PRIVATE RenderGraph
    PRIVATE Utils
)

add_test (NAME GVKTest
          COMMAND GVKTest
          WORKING_DIRECTORY bin)

add_custom_target (GVKTest_CopyTestFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TestData
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TestData
)
