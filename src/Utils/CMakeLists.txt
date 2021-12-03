
set (IncludePath ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set (HeadersPath ${CMAKE_CURRENT_SOURCE_DIR}/Include/Utils)
set (Headers
    ${HeadersPath}/Assert.hpp
    ${HeadersPath}/BuildType.hpp
    ${HeadersPath}/CommandLineFlag.hpp
    ${HeadersPath}/CompilerDefinitions.hpp
    ${HeadersPath}/Event.hpp
    ${HeadersPath}/GVKUtilsAPI.hpp
    ${HeadersPath}/Lazy.hpp
    ${HeadersPath}/MessageBox.hpp
    ${HeadersPath}/MovablePtr.hpp
    ${HeadersPath}/MultithreadedFunction.hpp
    ${HeadersPath}/NoInline.hpp
    ${HeadersPath}/Noncopyable.hpp
    ${HeadersPath}/Platform.hpp
    ${HeadersPath}/SourceLocation.hpp
    ${HeadersPath}/StaticInit.hpp
    ${HeadersPath}/TerminalColors.hpp
    ${HeadersPath}/Time.hpp
    ${HeadersPath}/Timer.hpp
    ${HeadersPath}/Utils.hpp
    ${HeadersPath}/FileSystemUtils.hpp
    ${HeadersPath}/UUID.hpp
    ${HeadersPath}/SetupLogger.hpp
)

set (SourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set (Sources
    ${SourcesPath}/Assert.cpp
    ${SourcesPath}/CommandLineFlag.cpp
    ${SourcesPath}/MessageBox.cpp
    ${SourcesPath}/SourceLocation.cpp
    ${SourcesPath}/Time.cpp
    ${SourcesPath}/Utils.cpp
    ${SourcesPath}/FileSystemUtils.cpp
    ${SourcesPath}/UUID.cpp
    ${SourcesPath}/SetupLogger.cpp
)

add_library (Utils SHARED ${Headers} ${Sources})

find_package (spdlog REQUIRED)

target_compile_definitions (Utils
    PUBLIC SPDLOG_FMT_EXTERNAL
)

target_precompile_headers (Utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeader.hpp)

target_include_directories (Utils
    PRIVATE ${HeadersPath}
    PUBLIC  ${IncludePath}

    PRIVATE ${fmt_INCLUDE_DIRS}
    PRIVATE ${spdlog_INCLUDE_DIRS}
)

if (WIN32)
    set (UUID_LIB rpcrt4)
elseif (UNIX)
    set (UUID_LIB uuid)
endif ()

target_link_libraries (Utils
    PRIVATE ${UUID_LIB}
    PRIVATE ${fmt_LIBRARIES}
    PRIVATE ${spdlog_LIBRARIES}
)
