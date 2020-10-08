@echo off

echo Updating git submodules...
git submodule update --init --recursive || goto :error

if not exist build.debug mkdir build.debug
pushd build.debug

echo Updating conan packages...
conan install .. --build=missing -s build_type=Debug 1> NUL || goto :error

echo Running CMake configuration...
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="Debug" -DCONAN_BUILDINFO_FILE="build.debug/conanbuildinfo.cmake" || goto :error

echo Compiling...
set CLICOLOR_FORCE=1
ninja -k 0 || goto :error

popd

echo Copying DLLs to python sources...
scripts\CopyGearsDLLDebug.bat 1> NUL || goto :error

:error
echo Failed with error #%errorlevel%.
popd
exit /b %errorlevel%