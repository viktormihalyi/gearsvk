@echo off

if not exist build.debug mkdir build.debug
pushd build.debug

echo Running CMake configuration...
set CC=cl
set CXX=cl
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="Debug" || goto :error

echo Compiling...
set CLICOLOR_FORCE=1
ninja -k 0 || goto :error

popd

copy /y build.debug\compile_commands.json compile_commands.json

:error
echo Failed with error #%errorlevel%.
popd
exit /b %errorlevel%