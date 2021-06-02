
set (SourcesPath ${CMAKE_SOURCE_DIR}/src/SequenceRunner/Sources)
set (Sources
    ${SourcesPath}/SequenceRunner.cpp
)

add_executable (SequenceRunner ${Headers} ${Sources})

target_compile_definitions (SequenceRunner PRIVATE SPDLOG_FMT_EXTERNAL)

target_include_directories (SequenceRunner
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
)

target_link_libraries (SequenceRunner
    PUBLIC Utils
    PUBLIC GVKRenderer
    PUBLIC GearsModuleEmbedded
)

install (TARGETS SequenceRunner
         RUNTIME DESTINATION bin)
