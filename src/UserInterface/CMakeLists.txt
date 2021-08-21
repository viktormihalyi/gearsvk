add_custom_target (UserInterface_CopyProjectFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

add_custom_target (UserInterface_CopyDLLsToSource ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Utils>
        $<TARGET_FILE:VulkanWrapper>
        $<TARGET_FILE:RenderGraph>
        $<TARGET_FILE:Sequence>
        $<TARGET_FILE:PySequence>
        $<TARGET_FILE:GearsModule>
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/
    DEPENDS Utils VulkanWrapper RenderGraph Sequence PySequence GearsModule
)

if (WIN32)
add_custom_target (UserInterface_CopyDLLsToSource_GearsModulePYD ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE_DIR:GearsModule>/$<TARGET_FILE_BASE_NAME:GearsModule>.pyd
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/
    DEPENDS GearsModule
)
endif ()
