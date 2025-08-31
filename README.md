# N-Body Gravity Simulation

N-Body Gravity Simulation made in C++ using the Qt framework and the DirectX 11 API.
Implemented using the Barnes-Hut algorithm. Capable of running up to 1M particles.

## How to Build

1. Create folder named "build" inside the root directory.
2. Run the following commands from said folder in the terminal (CMake and Qt 6 required):
- "cmake .." (for MinGW: "cmake .. -G "MinGW Makefiles"")
- "cmake --build ."
3. Run the executable.

## How to Use

The config.txt file is used to change three parameters at runtime:
- NR_PARTICLES - the number of particles simulated
- G - the gravitational constant
- THETA - threshold used for the Barnes-Hut implementation - determines the accuracy
of the algorithm (0.0 - brute-force, 1.0 - less accurate, but can simulate 1M particles
efficiently)

Modify the source code (specifically "main.cpp") for different particle formations.
Use SPACE to pause/unpause the simulation.
The framerate is displayed in the terminal.

## License

[LGPL 2.1](https://choosealicense.com/licenses/lgpl-2.1/)    