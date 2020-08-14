git submodule update --init --recursive
mkdir build.dev
cd build.dev
conan install .. --build=missing -s build_type=Debug
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="Debug" -DCONAN_BUILDINFO_FILE="build.dev/conanbuildinfo.cmake"
ninja
cd ..
