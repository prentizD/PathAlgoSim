CFLAGS = -O1 -Wall -std=c99 -Wno-missing-braces

ifeq ($(OS),Windows_NT)
	LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		LIBS = -lraylib -lGL -lm -lpthread -ldl -lrl -lx11
	endif
endif

default:
	gcc main.c -o simulation.exe $(CFLAGS) -I include/ -L lib/ $(LIBS)