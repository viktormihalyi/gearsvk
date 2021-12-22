add_executable (SequenceRunner)

target_sources (SequenceRunner
    PRIVATE
        Sources/SequenceRunner.cpp
        Sources/VideoExporter.cpp
)

find_package (ffmpeg REQUIRED)

target_include_directories (SequenceRunner
    PRIVATE
        SequenceRunner/Sources

        $<TARGET_PROPERTY:GearsModule,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
 
        ${ffmpeg_INCLUDE_DIRS}
)

target_link_libraries (SequenceRunner
    PRIVATE
        GearsModule
        Sequence
        RenderGraph
        Utils

        ${ffmpeg_LIBRARIES}
)
