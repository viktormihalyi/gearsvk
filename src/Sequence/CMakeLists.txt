add_library (Sequence SHARED)

set (Headers
    Include/Sequence/Pass.h
    Include/Sequence/Response.h
    Include/Sequence/Sequence.h
    Include/Sequence/SequenceAdapter.hpp
    Include/Sequence/SequenceAPI.hpp
    Include/Sequence/SpatialFilter.h
    Include/Sequence/Stimulus.h
    Include/Sequence/StimulusAdapter.hpp
    Include/Sequence/StimulusAdapterView.hpp
    
    Include/PySequence/core/PyPass.h
    Include/PySequence/core/PyStimulus.h
    Include/PySequence/core/PySequence.h
    Include/PySequence/core/PyResponse.h
    Include/PySequence/PyExtract.hpp
)

set (Sources
    Sources/Pass.cpp
    Sources/Response.cpp
    Sources/Sequence.cpp
    Sources/SequenceAdapter.cpp
    Sources/SpatialFilter.cpp
    Sources/Stimulus.cpp
    Sources/StimulusAdapter.cpp
    Sources/StimulusAdapterView.cpp
    
    Sources/event/events.cpp
    Sources/core/PyPass.cpp
    Sources/core/PyStimulus.cpp
    Sources/core/PySequence.cpp
    Sources/core/PyResponse.cpp
 
    Sources/LogInitializer.cpp
)

target_sources (Sequence PRIVATE ${Headers} ${Sources})

find_package (Python3 REQUIRED COMPONENTS Interpreter Development)
find_package (pybind11 REQUIRED)

target_precompile_headers (Sequence PRIVATE PrecompiledHeader.hpp)

target_include_directories (Sequence
    PRIVATE
        Include/Sequence
        Include/PySequence
        Include/PySequence/core
        Include/PySequence/event

    PUBLIC
        Include

        ${Python3_INCLUDE_DIRS}
        ${pybind11_INCLUDE_DIRS}

        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (Sequence
    PUBLIC
        ${Python3_LIBRARY_RELEASE}
        ${pybind11_LIBRARIES}
        RenderGraph
        Utils
)
