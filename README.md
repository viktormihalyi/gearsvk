# Building

Make sure to get the external libraries:
```
git submodule update --init
```
Also:
* Vulkan SDK: https://vulkan.lunarg.com/sdk/home
* Python 3 (64 bit)
* conan (`pip install conan`)

## Building on Windows with MSVC
You will need [CMake](https://cmake.org/download/) and the Developer Command Prompt (x64) for Visual Studio:

```
./BuildDebug.bat
```
or
```
./BuildRelease.bat
```

**Note:** You can also just open the project folder with Visual Studio and build from there.

## Building on Linux
For installing the Vulkan SDK on Ubuntu see: https://vulkan.lunarg.com/doc/sdk/latest/linux/getting_started_ubuntu.html

```
./BuildDebug.sh
```
or
```
./BuildRelease.sh
```
