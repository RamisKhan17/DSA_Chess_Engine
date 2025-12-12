@echo off
echo Compiling Chess Engine (MinGW g++)...

set SFML_INCLUDE=D:/C++ Libraries/SFML-3.0.2/include
set SFML_LIB=D:/C++ Libraries/SFML-3.0.2/lib

"g++" -std=c++17 -g -I./include src/Board.cpp src/GUI.cpp src/Evaluation.cpp src/Engine.cpp src/Search.cpp chess.cpp -lsfml-graphics -lsfml-window -lsfml-system -o chess

if %errorlevel%==0 (
  echo Compilation successful -> ChessEngine
) else (
  echo Compilation failed (code %errorlevel%)
  exit /b %errorlevel%
)
