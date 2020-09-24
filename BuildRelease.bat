git submodule update --init --recursive
mkdir build.release
cd build.release
conan install .. --build=missing -s build_type=Release
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -DCONAN_BUILDINFO_FILE="build.release/conanbuildinfo.cmake"
ninja
cd ..
scripts\CopyGearsDLLRelease.bat