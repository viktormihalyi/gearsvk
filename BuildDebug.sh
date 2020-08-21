git submodule update --init --recursive
mkdir build.debug
cd build.debug
conan install .. --build=missing -s build_type=Debug
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="Debug" -DCONAN_BUILDINFO_FILE="build.debug/conanbuildinfo.cmake"
ninja
cd ..
