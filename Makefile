COMPILER = g++
OUTPUT = PathAlgoSim.exe
FLAGS = -O1 -Wall -Wno-missing-braces
INCLUDE = -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm

default:
	g++ main.cpp -o $(OUTPUT) $(FLAGS) $(INCLUDE)