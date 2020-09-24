@echo off

setlocal

echo Updating git submodules...
git submodule update --init --recursive || goto :error

mkdir build.debug
cd build.debug

echo Updating conan packages...
conan install .. --build=missing -s build_type=Debug || goto :error

echo Running CMake configuration...
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="Debug" -DCONAN_BUILDINFO_FILE="build.debug/conanbuildinfo.cmake" || goto :error

echo Compiling...
ninja || goto :error

cd ..

echo Copying DLLs to python sources...
scripts\CopyGearsDLLDebug.bat || goto :error

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%