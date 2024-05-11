### Building

```
# On Windows
cmake -B build -G Ninja
ninja -C build install

# On Mac/Linux
cmake -B build -G Make
cd build
make install
```

This installs the game to `build/rogue`.
