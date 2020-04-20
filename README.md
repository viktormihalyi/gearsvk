# Building
```
git clone github.com/viktormihalyi/vulkantest
git submodule update --init
mkdir build
cd build
cmake ..
make
```
You may need to specify the python location for cmake:

`cmake .. -DPYTHON_EXECUTABLE=/usr/bin/python3.7`

# Running
`python3 src/UserInterface/Gears.py`