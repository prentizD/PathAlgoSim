# PathAlgoSim
This project is written in C and the goal is to visualize a variety of different algorithms (for example Dijkstra's algorithm for path finding).
For the visualization the raylib library is used:
https://www.raylib.com/
In short the raylib is a easy to use lightweighted game engine without the need of a integrated development environment.

## Setup/Build/Launch

Requirements:

- CMake Version >= 3.23.1 
- mingw32 (for Windows)
- make (it is called mingw32-make.exe under Windows with mingw32)
- gcc (using Linux)

## Linux:

cmake -S . -B build/
cd build && make && ./PathAlgoSim

## Windows:

cmake -S . -B build/ -G "MinGW Makefiles"
cd build && mingw32-make && PathAlgoSim

        
