
set (TARGET_NAME_UTILS GearsVkUtils)

add_library (${TARGET_NAME_UTILS} SHARED
    ${UTILS_SRC_FILES}
    src/Utils/Assert.cpp
    src/Utils/ImageLoader.cpp
    src/Utils/Logger.cpp
    src/Utils/MessageBox.cpp
    src/Utils/SourceLocation.cpp
    src/Utils/Utils.cpp
)

target_compile_definitions (${TARGET_NAME_UTILS} PRIVATE
    GEARSVK_UTILS_EXPORTS
    PROJECT_ROOT_FULL_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

target_include_directories (${TARGET_NAME_UTILS} PRIVATE
    "C:\\SDL2-2.0.10\\include"
)

target_link_directories (${TARGET_NAME_UTILS} PRIVATE
    "C:\\SDL2-2.0.10\\lib\\x64"
)

target_link_libraries (${TARGET_NAME_UTILS}
    SDL2
)