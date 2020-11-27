@echo off

echo Updating git submodules...
git submodule update --init --recursive || goto :error

if not exist build.release mkdir build.release
pushd build.release

echo Updating conan packages...
conan install .. --build=missing -s build_type=Release 1> NUL || goto :error

echo Running CMake configuration...
set CC=cl
set CXX=cl
set CXXFLAGS="/DFORCEDEBUGMODE /Od"
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -DCONAN_BUILDINFO_FILE="build.release/conanbuildinfo.cmake" 1> NUL || goto :error

echo Compiling...
set CLICOLOR_FORCE=1
ninja || goto :error

popd

echo Copying DLLs to python sources...
scripts\CopyGearsDLLRelease.bat 1> NUL || goto :error

:error
echo Failed with error #%errorlevel%.
popd
exit /b %errorlevel%