git submodule update --init --recursive
mkdir build.rel
cd build.rel
conan install .. --build=missing -s build_type=Release
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="Release" -DCONAN_BUILDINFO_FILE="build.rel/conanbuildinfo.cmake"
ninja
cd ..
