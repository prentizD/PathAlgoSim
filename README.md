# PathAlgoSim
This project is written in C and the goal is to visualize a variety of different algorithms (for example Dijkstra's algorithm for path finding).
For the visualization the raylib library is used:
https://www.raylib.com/
In short the raylib is a easy to use lightweighted game engine without the need of a integrated development environment.

## Setup/Build/Launch

Requirements:

Linux:
- gcc 
- install raylib (see: https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux) (Tested on V4.0 and V5.0)
- (linux requires only the \include directory and the main.c file)


Windows:
- mingw32 (for Windows)
(for windows V4.0 is used with the precompiled /lib/libraylib.a)

## Linux:

make
./build/simulation

## Windows:

gcc main.c -o simulation.exe -O1 -Wall -std=c99 -Wno-missing-braces -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
./simulation.exe
