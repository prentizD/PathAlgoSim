COMPILER = gcc
OUTPUT = PathAlgoSim.exe
FLAGS = -O1 -Wall -std=c99 -Wno-missing-braces
INCLUDE = -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm

default:
	$(COMPILER) main.c -o $(OUTPUT) $(FLAGS) $(INCLUDE)