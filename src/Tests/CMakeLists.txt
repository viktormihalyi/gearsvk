
set (SourcesPath ${CMAKE_SOURCE_DIR}/src/Tests/Sources)
set (Sources
    #${SourcesPath}/FontRenderingTests.cpp
    ${SourcesPath}/GearsTests.cpp
    ${SourcesPath}/GoogleTestEnvironment.hpp
    ${SourcesPath}/GoogleTestEnvironment.cpp
    ${SourcesPath}/RenderGraphTests.cpp
    ${SourcesPath}/TestMain.cpp
    ${SourcesPath}/VizHFTests.cpp
)

add_executable (GVKTest ${Headers} ${Sources})

find_package (GTest REQUIRED)

target_compile_definitions (GVKTest PRIVATE SPDLOG_FMT_EXTERNAL)

target_include_directories (GVKTest
    PUBLIC ${CMAKE_SOURCE_DIR}/src/Utils/Include/Utils
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Camera
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/RenderGraph
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/RenderGraph/DrawRecordable
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper/Utils
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Window

    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsPYD/Include/GearsPYD

    PUBLIC ${SourcesPath}

    PUBLIC ${GTest_INCLUDE_DIRS}
)

target_link_libraries (GVKTest
    PUBLIC Utils
    PUBLIC GVKRenderer
    PUBLIC GearsModuleEmbedded
    PUBLIC ${GTest_LIBRARIES}
)
