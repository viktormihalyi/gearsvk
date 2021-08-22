
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/GearsPYD)
set (Headers
    ${HeadersPath}/GearsModuleCommon.hpp
    ${HeadersPath}/GearsAPIv2.hpp
)

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/GearsAPIv2.cpp
    ${SourcesPath}/GearsModule.cpp
)

add_library (GearsModule SHARED ${Headers} ${Sources})

target_include_directories (GearsModule
    PRIVATE ${HeadersPath}

    PUBLIC 
        ${IncludePath}

        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (GearsModule
    PUBLIC Sequence
    PUBLIC RenderGraph
    PUBLIC Utils
)

if (WIN32)
add_custom_command (TARGET GearsModule POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:GearsModule>
        $<TARGET_FILE_DIR:GearsModule>/$<TARGET_FILE_BASE_NAME:GearsModule>.pyd
    COMMENT "Renaming GearsModule.dll to GearsModule.pyd..."
)
endif ()
