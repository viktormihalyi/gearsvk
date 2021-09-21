# Building

**Dependencies**

* Vulkan SDK - https://vulkan.lunarg.com/sdk/home
* Python 3 (64 bit)
* conan - for package management https://conan.io/index.html (`pip install conan`)

**Windows MSVC**

Open Developer Command Prompt for VS

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

**Linux**

Make sure to set `settings.compiler.libcxx=libstdc++11` in a conan profile.

Update the default profile
```
conan profile new default --detect --force
conan profile update settings.compiler.libcxx=libstdc++11 default
```

Then

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```
