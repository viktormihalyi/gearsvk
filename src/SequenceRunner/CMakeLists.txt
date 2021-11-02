
add_executable (SequenceRunner
    Sources/SequenceRunner.cpp
    Sources/VideoExporter.cpp
)

find_package (ffmpeg REQUIRED)

target_include_directories (SequenceRunner
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SequenceRunner/Sources

    PRIVATE $<TARGET_PROPERTY:GearsModule,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
 
    PRIVATE ${ffmpeg_INCLUDE_DIRS}
)

target_link_libraries (SequenceRunner
    PRIVATE GearsModule
    PRIVATE Sequence
    PRIVATE RenderGraph
    PRIVATE Utils

    PRIVATE ${ffmpeg_LIBRARIES}
)
