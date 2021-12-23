add_library (Utils SHARED)

set (Headers
    Include/Utils/Assert.hpp
    Include/Utils/BuildType.hpp
    Include/Utils/CommandLineFlag.hpp
    Include/Utils/CompilerDefinitions.hpp
    Include/Utils/Event.hpp
    Include/Utils/GVKUtilsAPI.hpp
    Include/Utils/Lazy.hpp
    Include/Utils/MessageBox.hpp
    Include/Utils/MovablePtr.hpp
    Include/Utils/MultithreadedFunction.hpp
    Include/Utils/NoInline.hpp
    Include/Utils/Noncopyable.hpp
    Include/Utils/Platform.hpp
    Include/Utils/SourceLocation.hpp
    Include/Utils/StaticInit.hpp
    Include/Utils/TerminalColors.hpp
    Include/Utils/Time.hpp
    Include/Utils/Timer.hpp
    Include/Utils/Utils.hpp
    Include/Utils/FileSystemUtils.hpp
    Include/Utils/UUID.hpp
    Include/Utils/SetupLogger.hpp
)

set (Sources
    Sources/Assert.cpp
    Sources/CommandLineFlag.cpp
    Sources/MessageBox.cpp
    Sources/SourceLocation.cpp
    Sources/Time.cpp
    Sources/Utils.cpp
    Sources/FileSystemUtils.cpp
    Sources/UUID.cpp
    Sources/SetupLogger.cpp
)

target_sources (Utils PRIVATE ${Headers} ${Sources})

target_compile_definitions (Utils
    PUBLIC SPDLOG_FMT_EXTERNAL
)

target_precompile_headers (Utils PRIVATE PrecompiledHeader.hpp)

FetchContent_Declare (
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        eb3220622e73a4889eee355ffa37972b3cac3df5 # v1.9.2
)

FetchContent_Declare (
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        d141cdbeb0fb422a3fb7173b285fd38e0d1772dc # 8.0.1
)

FetchContent_MakeAvailable (fmt spdlog)

target_include_directories (Utils
    PRIVATE
        Include/Utils

    PUBLIC
        Include
        $<TARGET_PROPERTY:fmt::fmt,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:spdlog::spdlog,INTERFACE_INCLUDE_DIRECTORIES>
)

if (WIN32)
    set (UUID_LIB rpcrt4)
elseif (UNIX)
    set (UUID_LIB uuid)
endif ()

target_link_libraries (Utils
    PRIVATE
        ${UUID_LIB}
    PUBLIC
        fmt::fmt
)
