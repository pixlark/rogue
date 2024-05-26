### Building

```
# with go-task installed
task build

# to quickly run with go-task
task run

# to build without go-task
cmake -B build -G Ninja
cmake --build build --target install

# to run without go-task (after building)
./build/rogue/bin/core     # on linux
.\build\rogue\bin\core.exe # on windows
```

Building the `install` target is essential here. It does not install to any system paths, it installs all the build artifacts into `build/rogue`, in the correct places so that everything can run properly.

Note: On Windows make sure you are building from a visual studio developer prompt. Otherwise CMake will not be able to find the MSVC compiler tools.
