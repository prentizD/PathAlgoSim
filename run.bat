@echo off
echo INFO: Compiling with g++ ...
g++ main.cpp -o PathAlgoSim.exe -O1 -Wall -Wno-missing-braces -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm

::EXE nur starten, falls kein compile error
IF %ERRORLEVEL% NEQ 0 (
    echo ERROR: COMPILE ERROR
) ELSE (
    .\PathAlgoSim.exe
)