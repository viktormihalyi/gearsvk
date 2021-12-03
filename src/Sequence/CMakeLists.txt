
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (Headers
    ${IncludePath}/Sequence/Pass.h
    ${IncludePath}/Sequence/Response.h
    ${IncludePath}/Sequence/Sequence.h
    ${IncludePath}/Sequence/SequenceAdapter.hpp
    ${IncludePath}/Sequence/SequenceAPI.hpp
    ${IncludePath}/Sequence/SpatialFilter.h
    ${IncludePath}/Sequence/Stimulus.h
    ${IncludePath}/Sequence/StimulusAdapter.hpp
    ${IncludePath}/Sequence/StimulusAdapterView.hpp
    
    ${IncludePath}/PySequence/core/PyPass.h
    ${IncludePath}/PySequence/core/PyStimulus.h
    ${IncludePath}/PySequence/core/PySequence.h
    ${IncludePath}/PySequence/core/PyResponse.h
    ${IncludePath}/PySequence/PyExtract.hpp
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
    
    ${SourcesPath}/event/events.cpp
    ${SourcesPath}/core/PyPass.cpp
    ${SourcesPath}/core/PyStimulus.cpp
    ${SourcesPath}/core/PySequence.cpp
    ${SourcesPath}/core/PyResponse.cpp
 
    ${SourcesPath}/LogInitializer.cpp
)

add_library (Sequence SHARED ${Headers} ${Sources})

find_package (Python3 REQUIRED COMPONENTS Interpreter Development)
find_package (pybind11 REQUIRED)

target_precompile_headers (Sequence PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeader.hpp)

target_include_directories (Sequence
    PRIVATE
        ${IncludePath}
        ${IncludePath}/Sequence
        ${IncludePath}/PySequence
        ${IncludePath}/PySequence/core
        ${IncludePath}/PySequence/event

    PUBLIC
        ${IncludePath}

        ${Python3_INCLUDE_DIRS}
        ${pybind11_INCLUDE_DIRS}

        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (Sequence
    PUBLIC ${Python3_LIBRARY_RELEASE}
    PUBLIC ${pybind11_LIBRARIES}
    PUBLIC RenderGraph
    PUBLIC Utils
)
