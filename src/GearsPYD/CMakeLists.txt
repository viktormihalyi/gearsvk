add_library (GearsModule SHARED)

set (Headers
    Include/GearsPYD/GearsModuleCommon.hpp
    Include/GearsPYD/GearsAPIv2.hpp
)

set (Sources
    Sources/GearsAPIv2.cpp
    Sources/GearsModule.cpp
    Sources/LogInitializer.cpp
)

target_sources (GearsModule PRIVATE ${Headers} ${Sources})

target_precompile_headers (GearsModule PRIVATE PrecompiledHeader.hpp)

target_include_directories (GearsModule
    PRIVATE
        Include/GearsPYD

    PUBLIC 
        Include

        $<TARGET_PROPERTY:Utils,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:RenderGraph,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Sequence,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries (GearsModule
    PUBLIC
        Sequence
        RenderGraph
        Utils
)

if (WIN32)
add_custom_command (TARGET GearsModule POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:GearsModule>
        $<TARGET_FILE_DIR:GearsModule>/$<TARGET_FILE_BASE_NAME:GearsModule>.pyd
    COMMENT "Renaming GearsModule.dll to GearsModule.pyd..."
)
endif ()
