
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/GearsPYD)
set (Headers
    ${HeadersPath}/core/PythonDict.h
    ${HeadersPath}/core/PyPass.h
    ${HeadersPath}/core/PyStimulus.h
    ${HeadersPath}/core/PySequence.h
    ${HeadersPath}/core/PyResponse.h
    ${HeadersPath}/PyExtract.hpp

    ${HeadersPath}/GearsAPIv2.hpp
)

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/event/events.cpp

    ${SourcesPath}/core/PyPass.cpp
    ${SourcesPath}/core/PyStimulus.cpp
    ${SourcesPath}/core/PySequence.cpp
    ${SourcesPath}/core/PyResponse.cpp

    ${SourcesPath}/GearsAPIv2.cpp
)

add_library (GearsModule SHARED ${Headers} ${Sources} ${SourcesPath}/GearsModule.cpp)
add_library (GearsModuleEmbedded SHARED ${Headers} ${Sources} ${SourcesPath}/GearsModuleEmbedded.cpp)

find_package (Python3 REQUIRED COMPONENTS Interpreter Development)
find_package (pybind11 REQUIRED)

set (IncludeDirectoriesPrivate
    ${HeadersPath}
    ${HeadersPath}/core
    ${HeadersPath}/core/filter
    ${HeadersPath}/core/filter/fft
    ${HeadersPath}/curve
    ${HeadersPath}/event
    ${HeadersPath}/font
    ${HeadersPath}/gpu
    ${HeadersPath}/video
)

set (IncludeDirectoriesPublic
    ${IncludePath}

    $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>

    ${Python3_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
)

target_precompile_headers (GearsModule PRIVATE ${SourcesPath}/PCH.hpp)
target_precompile_headers (GearsModuleEmbedded PRIVATE ${SourcesPath}/PCH.hpp)


target_include_directories (GearsModule
    PRIVATE ${IncludeDirectoriesPrivate}
    PUBLIC ${IncludeDirectoriesPublic}
)

target_include_directories (GearsModuleEmbedded
    PRIVATE ${IncludeDirectoriesPrivate}
    PUBLIC ${IncludeDirectoriesPublic}
)

target_link_libraries (GearsModule
    PUBLIC Utils
    PUBLIC RenderGraph
    PUBLIC Sequence
    PUBLIC ${Python3_LIBRARIES}
    PUBLIC ${pybind11_LIBRARIES}
)

target_link_libraries (GearsModuleEmbedded
    PUBLIC Utils
    PUBLIC RenderGraph
    PUBLIC Sequence
    PUBLIC ${Python3_LIBRARIES}
    PUBLIC ${pybind11_LIBRARIES}
)

install (TARGETS GearsModule
         RUNTIME DESTINATION bin)

install (TARGETS GearsModuleEmbedded
         RUNTIME DESTINATION bin)
