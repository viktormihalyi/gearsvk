
set (IncludePath ${CMAKE_SOURCE_DIR}/src/GearsPYD/Include)
set (HeadersPath ${CMAKE_SOURCE_DIR}/src/GearsPYD/Include/GearsPYD)
set (Headers
    ${HeadersPath}/core/PythonDict.h
    ${HeadersPath}/core/PyPass.h
    ${HeadersPath}/core/PyStimulus.h
    ${HeadersPath}/core/PySequence.h
    ${HeadersPath}/core/PyResponse.h
    ${HeadersPath}/PyExtract.hpp

    ${HeadersPath}/GearsAPIv2.hpp
)

set (SourcesPath ${CMAKE_SOURCE_DIR}/src/GearsPYD/Sources)
set (Sources
    ${SourcesPath}/event/events.cpp

    ${SourcesPath}/core/PyPass.cpp
    ${SourcesPath}/core/PyStimulus.cpp
    ${SourcesPath}/core/PySequence.cpp
    ${SourcesPath}/core/PyResponse.cpp

    ${SourcesPath}/GearsAPIv2.cpp

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

    ${CMAKE_SOURCE_DIR}/src/Utils/Include

    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Camera
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/RenderGraph
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/VulkanWrapper/Utils
    ${CMAKE_SOURCE_DIR}/src/GearsVk/Include/GearsVk/Window
    
    ${CMAKE_SOURCE_DIR}/src/Sequence/Include/Sequence

    ${Python3_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
)

target_precompile_headers (GearsModule PRIVATE [["PCH.hpp"]])
target_precompile_headers (GearsModuleEmbedded PRIVATE [["PCH.hpp"]])


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
           ${pybind11_LIBRARIES}
)

target_link_libraries (GearsModuleEmbedded
    PUBLIC Utils
    PUBLIC RenderGraph
    PUBLIC Sequence
    PUBLIC ${Python3_LIBRARIES}
           ${pybind11_LIBRARIES}
)

install (TARGETS GearsModule
         RUNTIME DESTINATION bin)

install (TARGETS GearsModuleEmbedded
         RUNTIME DESTINATION bin)
