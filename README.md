# Building

Make sure to get the external libraries:
```
git submodule update --init
```
Also:
* Vulkan SDK: https://vulkan.lunarg.com/sdk/home
* Python 3

## Building on Windows with MSVC
You will need [CMake](https://cmake.org/download/) and the Developer Command Prompt for Visual Studio:

```
mkdir build
cd build
cmake ..
msbuild ALL_BUILD.vcxproj -property:Configuration=Release
```

**Note:** For building only specific components replace `ALL_BUILD` with the components name.

**Note:** You can also just open the project folder with Visual Studio and build from there.

## Building on Linux
For installing the Vulkan SDK on Ubuntu see: https://vulkan.lunarg.com/doc/sdk/latest/linux/getting_started_ubuntu.html

On Ubuntu you will need these packages for compiling GLFW: `libxinerama-dev libxcursor-dev libxi-dev`

```
mkdir build
cd build
cmake ..
make
```

**Note:** For building only specific components type name of the component after `make`.

**Note:** You may need to specify the Python location for CMake:
```
cmake .. -DPYTHON_EXECUTABLE=/usr/bin/python3.7
```
