
set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    #${SourcesPath}/FontRenderingTests.cpp
    ${SourcesPath}/TestMain.cpp
    ${SourcesPath}/GoogleTestEnvironment.hpp
    ${SourcesPath}/GoogleTestEnvironment.cpp
    ${SourcesPath}/RenderGraphTests.cpp
    ${SourcesPath}/VizHFTests.cpp
    ${SourcesPath}/GearsTests.cpp
)

add_executable (GVKTest ${Sources})

find_package (GTest REQUIRED)

target_include_directories (GVKTest
    PRIVATE $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:PySequence,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:GearsModuleEmbedded,INTERFACE_INCLUDE_DIRECTORIES>

    PRIVATE ${SourcesPath}

    PRIVATE ${GTest_INCLUDE_DIRS}
)

target_link_libraries (GVKTest
    PRIVATE Utils
    PRIVATE RenderGraph
    PRIVATE Sequence
    PRIVATE PySequence
    PRIVATE GearsModuleEmbedded
    PRIVATE ${GTest_LIBRARIES}
)

file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TestData
      DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
