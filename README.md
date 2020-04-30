# Building
```
git clone github.com/viktormihalyi/gearsvk
cd gearsvk
git submodule update --init
mkdir build
cd build
cmake ..
make
```

You may need to specify the Python location for cmake:
```
cmake .. -DPYTHON_EXECUTABLE=/usr/bin/python3.7
```
