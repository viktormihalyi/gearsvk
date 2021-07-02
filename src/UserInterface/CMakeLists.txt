add_custom_target (CopyProjectFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
