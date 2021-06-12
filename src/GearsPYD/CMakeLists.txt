
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/GearsPYD)
set (Headers
    ${HeadersPath}/GearsModuleCommon.hpp
    ${HeadersPath}/GearsAPIv2.hpp
)

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/GearsAPIv2.cpp
)

add_library (GearsModule SHARED ${Headers} ${Sources} ${SourcesPath}/GearsModule.cpp)
add_library (GearsModuleEmbedded SHARED ${Headers} ${Sources} ${SourcesPath}/GearsModuleEmbedded.cpp)

set (IncludeDirectoriesPrivate
    ${HeadersPath}
)

set (IncludeDirectoriesPublic
    ${IncludePath}

    $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:PySequence,INTERFACE_INCLUDE_DIRECTORIES>
)

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
    PUBLIC PySequence
)

target_link_libraries (GearsModuleEmbedded
    PUBLIC Utils
    PUBLIC RenderGraph
    PUBLIC Sequence
    PUBLIC PySequence
)
