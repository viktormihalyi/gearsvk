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

target_precompile_headers (Sequence PRIVATE PrecompiledHeader.hpp)

FetchContent_Declare (
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        f7b499615e14d70ab098a20deb0cdb3889998a1a # v2.8.1
)

FetchContent_MakeAvailable (pybind11)

target_include_directories (Sequence
    PRIVATE
        Include/Sequence
        Include/PySequence
        Include/PySequence/core
        Include/PySequence/event

    PUBLIC
        Include

        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
 
        $<TARGET_PROPERTY:pybind11::pybind11,INTERFACE_INCLUDE_DIRECTORIES>
        ${Python3_INCLUDE_DIRS}

)

target_link_libraries (Sequence
    PUBLIC
        Utils
        RenderGraph
        pybind11::pybind11
        ${Python3_LIBRARY_RELEASE}
)
