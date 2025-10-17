# PowerShell script to compile Chess Engine using MinGW g++
# Simplified: no CMake, no MSVC paths, no prompts

Write-Host "Compiling Chess Engine (MinGW g++)" -ForegroundColor Green

# Adjust these if your SFML path changes
$sfmlInclude = "D:/C++ Libraries/SFML-3.0.2/include"
$sfmlLib = "D:/C++ Libraries/SFML-3.0.2/lib"

$argsList = @(
    "-std=c++17",
    "-I./include",
    "-I$sfmlInclude",
    "-L$sfmlLib",
    "src/Board.cpp",
    "src/GUI.cpp",
    "main.cpp",
    "-lsfml-graphics",
    "-lsfml-window",
    "-lsfml-system",
    "-o",
    "ChessEngine"
)

& "g++" @argsList

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful -> ChessEngine" -ForegroundColor Green
} else {
    Write-Host "Compilation failed (code $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}
