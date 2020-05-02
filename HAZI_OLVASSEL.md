A `screenshots` mappába tettem képeket a végeredményről ha nem sikerülne fordítani vagy futtatni. A releváns shader a `src/VizHF/shaders` mappában található.

Elérhető git repoként is ha baj lenne a beadott változattal: https://github.com/viktormihalyi/gearsvk

# Fordítás

Szükség lesz a Vulkan SDK-ra is: https://vulkan.lunarg.com/sdk/home

## a) Windowson Visual Studioval
A projekt mappát megnyitva a `VizHF` nevű targetet kell fordítani/futtatni. (Ehhez szükség van a C++ CMake tools-ra az installerből.)

## b) Windowson CMake-el, Developer Command Prompt-ból
```
mkdir build
cd build
cmake ..
msbuild VizHF.vcxproj -property:Configuration=Release
```
A `VizHF.exe` a `build/Release` mappában lesz.

## c) Linuxon
Ubuntun szükség lesz ezekre a packagekre is (GLFW-hez): `libxinerama-dev libxcursor-dev libxi-dev`

```
mkdir build
cd build
cmake ..
make VizHF
```

# Használat

WASDEQ, egér: fps kamera mozgatás

1-5: váltás a megoldott feladatok között (sorrendben Befoglaló, Szintfelület, Matcap, Hagymahéj, Önárnyék)
