@echo off

if not exist build.release mkdir build.release
pushd build.release

echo Running CMake configuration...
set CC=cl
set CXX=cl
set CXXFLAGS="/DFORCEDEBUGMODE /Od"
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" 1> NUL || goto :error

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