
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/Sequence)
set (Headers
    ${HeadersPath}/Pass.h
    ${HeadersPath}/Response.h
    ${HeadersPath}/Sequence.h
    ${HeadersPath}/SequenceAdapter.hpp
    ${HeadersPath}/SequenceAPI.hpp
    ${HeadersPath}/SpatialFilter.h
    ${HeadersPath}/Stimulus.h
    ${HeadersPath}/StimulusAdapter.hpp
    ${HeadersPath}/StimulusAdapterView.hpp
)

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/Pass.cpp
    ${SourcesPath}/Response.cpp
    ${SourcesPath}/Sequence.cpp
    ${SourcesPath}/SequenceAdapter.cpp
    ${SourcesPath}/SpatialFilter.cpp
    ${SourcesPath}/Stimulus.cpp
    ${SourcesPath}/StimulusAdapter.cpp
    ${SourcesPath}/StimulusAdapterView.cpp
)

add_library (Sequence SHARED ${Headers} ${Sources})

target_include_directories (Sequence
    PRIVATE ${HeadersPath}
    PUBLIC ${IncludePath}

    PUBLIC $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (Sequence
    PUBLIC Utils
    PUBLIC RenderGraph
)
