
set (SourcesPath ${CMAKE_SOURCE_DIR}/src/Tests/Sources)
set (Sources
    #${SourcesPath}/FontRenderingTests.cpp
    ${SourcesPath}/TestMain.cpp
    ${SourcesPath}/GoogleTestEnvironment.hpp
    ${SourcesPath}/GoogleTestEnvironment.cpp
    ${SourcesPath}/RenderGraphTests.cpp
    ${SourcesPath}/VizHFTests.cpp
    ${SourcesPath}/GearsTests.cpp
)

add_executable (GVKTest ${Headers} ${Sources})

find_package (GTest REQUIRED)

target_compile_definitions (GVKTest PRIVATE SPDLOG_FMT_EXTERNAL)

target_include_directories (GVKTest
    PUBLIC ${CMAKE_SOURCE_DIR}/src/Utils/Include

    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Camera
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/RenderGraph
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper/Utils
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Window

    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsPYD/Include/GearsPYD

    PUBLIC ${SourcesPath}

    PUBLIC ${GTest_INCLUDE_DIRS}
)

target_link_libraries (GVKTest
    PUBLIC Utils
    PUBLIC RenderGraph
    PUBLIC GearsModuleEmbedded
    PUBLIC ${GTest_LIBRARIES}
)

target_link_directories (GVKTest
    PUBLIC ${CMAKE_BINARY_DIR}/src/Utils
    PUBLIC ${CMAKE_BINARY_DIR}/src/GearsVk
    PUBLIC ${CMAKE_BINARY_DIR}/src/GearsPYD
)

file (COPY ${CMAKE_SOURCE_DIR}/src/Tests/Sources/TestData
      DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS GVKTest
         RUNTIME DESTINATION bin)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/src/Tests/Sources/TestData DESTINATION bin)
