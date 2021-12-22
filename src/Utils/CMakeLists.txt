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

find_package (spdlog REQUIRED)

target_compile_definitions (Utils
    PUBLIC SPDLOG_FMT_EXTERNAL
)

target_precompile_headers (Utils PRIVATE PrecompiledHeader.hpp)

target_include_directories (Utils
    PRIVATE
        Include/Utils
        ${fmt_INCLUDE_DIRS}
        ${spdlog_INCLUDE_DIRS}

    PUBLIC
        Include
)

if (WIN32)
    set (UUID_LIB rpcrt4)
elseif (UNIX)
    set (UUID_LIB uuid)
endif ()

target_link_libraries (Utils
    PRIVATE
        ${UUID_LIB}
        ${fmt_LIBRARIES}
        ${spdlog_LIBRARIES}
)
