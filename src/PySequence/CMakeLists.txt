
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/PySequence)
set (Headers
    ${HeadersPath}/core/PyPass.h
    ${HeadersPath}/core/PyStimulus.h
    ${HeadersPath}/core/PySequence.h
    ${HeadersPath}/core/PyResponse.h
    ${HeadersPath}/PySequenceAPI.hpp
    ${HeadersPath}/PyExtract.hpp
)

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/event/events.cpp
    ${SourcesPath}/core/PyPass.cpp
    ${SourcesPath}/core/PyStimulus.cpp
    ${SourcesPath}/core/PySequence.cpp
    ${SourcesPath}/core/PyResponse.cpp
)

add_library (PySequence SHARED ${Headers} ${Sources})

find_package (Python3 REQUIRED COMPONENTS Interpreter Development)
find_package (pybind11 REQUIRED)

target_precompile_headers (PySequence PRIVATE ${SourcesPath}/PCH.hpp)

target_include_directories (PySequence
    PRIVATE
        ${HeadersPath}
        ${HeadersPath}/core
        ${HeadersPath}/event

    PUBLIC
        ${IncludePath}

        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>

        ${Python3_INCLUDE_DIRS}
        ${pybind11_INCLUDE_DIRS}
)

target_link_libraries (PySequence
    PUBLIC Utils
    PUBLIC RenderGraph
    PUBLIC Sequence
    PUBLIC ${Python3_LIBRARIES}
    PUBLIC ${pybind11_LIBRARIES}
)
