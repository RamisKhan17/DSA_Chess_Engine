@echo off
echo Compiling Chess Engine (MinGW g++)...

set SFML_INCLUDE=D:/C++ Libraries/SFML-3.0.2/include
set SFML_LIB=D:/C++ Libraries/SFML-3.0.2/lib

"g++" -std=c++17 -g -I./include src/Board.cpp src/GUI.cpp src/Engine.cpp main.cpp -lsfml-graphics -lsfml-window -lsfml-system -o ChessEngine

if %errorlevel%==0 (
  echo Compilation successful -> ChessEngine
) else (
  echo Compilation failed (code %errorlevel%)
  exit /b %errorlevel%
)
