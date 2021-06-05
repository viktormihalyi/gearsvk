
set (SourcesPath ${CMAKE_SOURCE_DIR}/src/SequenceRunner/Sources)
set (Sources
    ${SourcesPath}/SequenceRunner.cpp
)

add_executable (SequenceRunner ${Headers} ${Sources})

target_compile_definitions (SequenceRunner PRIVATE SPDLOG_FMT_EXTERNAL)

target_include_directories (SequenceRunner

    PUBLIC ${CMAKE_SOURCE_DIR}/src/Utils/Include
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsPYD/Include

    PUBLIC ${SourcesPath}
)

target_link_libraries (SequenceRunner
    PUBLIC Utils
    PUBLIC RenderGraph
    PUBLIC GearsModuleEmbedded
)

install (TARGETS SequenceRunner
         RUNTIME DESTINATION bin)
