
set (IncludePath ${CMAKE_SOURCE_DIR}/src/GearsPYD/Include)
set (HeadersPath ${CMAKE_SOURCE_DIR}/src/GearsPYD/Include/GearsPYD)
set (Headers
    ${HeadersPath}/core/PythonDict.h
    ${HeadersPath}/core/Pass.h
    ${HeadersPath}/core/PassRenderer.h
    ${HeadersPath}/core/Stimulus.h
    ${HeadersPath}/core/StimulusRenderer.h
    ${HeadersPath}/core/Sequence.h
    ${HeadersPath}/core/SequenceRenderer.h

    ${HeadersPath}/GearsAPIv2.hpp
    ${HeadersPath}/StimulusAdapter.hpp
    ${HeadersPath}/StimulusAdapterView.hpp
    ${HeadersPath}/SequenceAdapter.hpp
)

set (SourcesPath ${CMAKE_SOURCE_DIR}/src/GearsPYD/Sources)
set (Sources
    ${SourcesPath}/event/events.cpp

    ${SourcesPath}/core/Pass.cpp
    ${SourcesPath}/core/PassRenderer.cpp
    ${SourcesPath}/core/Stimulus.cpp
    ${SourcesPath}/core/StimulusRenderer.cpp
    ${SourcesPath}/core/Sequence.cpp
    ${SourcesPath}/core/SequenceRenderer.cpp
    ${SourcesPath}/core/Response.cpp
    ${SourcesPath}/core/Ticker.cpp
    ${SourcesPath}/core/PortHandlerWin32.cpp
    ${SourcesPath}/core/PortHandlerLinux.cpp
    ${SourcesPath}/core/filter/KernelManager.cpp
    ${SourcesPath}/core/filter/SpatialFilter.cpp

    ${SourcesPath}/gpu/Framebuffer.cpp
    ${SourcesPath}/gpu/Nothing.cpp
    ${SourcesPath}/gpu/Pointgrid.cpp
    ${SourcesPath}/gpu/Quad.cpp
    ${SourcesPath}/gpu/RandomSequenceBuffer.cpp
    ${SourcesPath}/gpu/Shader.cpp
    ${SourcesPath}/gpu/StimulusGrid.cpp
    ${SourcesPath}/gpu/Texture.cpp
    ${SourcesPath}/gpu/TextureQueue.cpp

    ${SourcesPath}/GearsAPIv2.cpp
    ${SourcesPath}/StimulusAdapter.cpp
    ${SourcesPath}/StimulusAdapterView.cpp
    ${SourcesPath}/SequenceAdapter.cpp

    ${CMAKE_SOURCE_DIR}/src/GearsVk/Sources/VulkanMemoryAllocatorImpl.cpp #???
)

add_library (GearsModule SHARED ${Headers} ${Sources} ${SourcesPath}/GearsModule.cpp)
add_library (GearsModuleEmbedded SHARED ${Headers} ${Sources} ${SourcesPath}/GearsModuleEmbedded.cpp)

find_package (Python3 REQUIRED COMPONENTS Interpreter Development)
find_package (pybind11 REQUIRED)

target_compile_definitions (GearsModule PRIVATE GEARS_EXPORTS SPDLOG_FMT_EXTERNAL)
target_compile_definitions (GearsModuleEmbedded PRIVATE GEARS_EMBEDDED_EXPORTS SPDLOG_FMT_EXTERNAL)

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

    ${CMAKE_SOURCE_DIR}/src/Utils/Include/Utils
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Camera
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/RenderGraph
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/RenderGraph/DrawRecordable
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper/Utils
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Window

    ${Python3_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
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
    PUBLIC GVKRenderer
    PUBLIC ${Python3_LIBRARIES}
           ${pybind11_LIBRARIES}
)

target_link_libraries (GearsModuleEmbedded
    PUBLIC Utils
    PUBLIC GVKRenderer
    PUBLIC ${Python3_LIBRARIES}
           ${pybind11_LIBRARIES}
)

install (TARGETS GearsModule
         RUNTIME DESTINATION bin)

install (FILES $<TARGET_PDB_FILE:GearsModule> DESTINATION bin OPTIONAL)

install (TARGETS GearsModuleEmbedded
         RUNTIME DESTINATION bin)

install (FILES $<TARGET_PDB_FILE:GearsModuleEmbedded> DESTINATION bin OPTIONAL)


