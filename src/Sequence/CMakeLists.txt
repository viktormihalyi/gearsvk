
set (IncludePath ${CMAKE_SOURCE_DIR}/src/Sequence/Include)
set (HeadersPath ${CMAKE_SOURCE_DIR}/src/Sequence/Include/Sequence)
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

set (SourcesPath ${CMAKE_SOURCE_DIR}/src/Sequence/Sources)
set (Sources
    ${SourcesPath}/Pass.cpp
    ${SourcesPath}/Response.cpp
    ${SourcesPath}/Sequence.cpp
    ${SourcesPath}/SequenceAdapter.cpp
    ${SourcesPath}/SpatialFilter.cpp
    ${SourcesPath}/Stimulus.cpp
    ${SourcesPath}/StimulusAdapter.cpp
    ${SourcesPath}/StimulusAdapterView.cpp

    ${CMAKE_SOURCE_DIR}/src/GearsVk/Sources/VulkanMemoryAllocatorImpl.cpp #???
)

add_library (Sequence SHARED ${Headers} ${Sources})

target_compile_definitions (Sequence PRIVATE Sequence_EXPORTS SPDLOG_FMT_EXTERNAL)

target_include_directories (Sequence
    PRIVATE ${HeadersPath}
    PUBLIC ${IncludePath}

    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk

    PUBLIC ${CMAKE_SOURCE_DIR}/src/Utils/Include

    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Camera
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/RenderGraph
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper/Utils
    PUBLIC ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Window
)

target_link_libraries (Sequence
    PUBLIC Utils
    PUBLIC GVKRenderer
)

install (TARGETS Sequence
         RUNTIME DESTINATION bin)
