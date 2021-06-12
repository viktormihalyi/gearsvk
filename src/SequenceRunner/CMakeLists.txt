
add_executable (SequenceRunner
    Sources/SequenceRunner.cpp
)

target_include_directories (SequenceRunner
    PRIVATE $<TARGET_PROPERTY:GearsModuleEmbedded,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:PySequence,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (SequenceRunner
    PRIVATE GearsModuleEmbedded
    PRIVATE PySequence
    PRIVATE Sequence
    PRIVATE RenderGraph
    PRIVATE Utils
)
