
add_executable (SequenceRunner
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/SequenceRunner.cpp
)

target_include_directories (SequenceRunner
    PRIVATE $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:GearsModuleEmbedded,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (SequenceRunner
    PRIVATE Utils
    PRIVATE RenderGraph
    PRIVATE GearsModuleEmbedded
)

install (TARGETS SequenceRunner
         RUNTIME DESTINATION bin)
