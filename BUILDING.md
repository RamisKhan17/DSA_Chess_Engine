# Building the Chess Engine

This guide will help you compile and run the chess engine on your system.

## Prerequisites

### Required Software

1. **C++ Compiler** with C++17 support:
   - Windows: MinGW-w64, Visual Studio 2017+, or Clang
   - Linux: GCC 7+ or Clang 5+
   - macOS: Clang (via Xcode Command Line Tools)

2. **CMake** version 3.10 or higher
   - Download from: https://cmake.org/download/

3. **SFML** version 2.5 or higher (Simple and Fast Multimedia Library)
   - This is the MOST IMPORTANT dependency

## Installing SFML

### Windows

**Option 1: Manual Installation (Recommended for this project)**

1. Download SFML from: https://www.sfml-dev.org/download.php
   - Choose the version matching your compiler (MinGW or Visual C++)
   - For MinGW: Download "GCC X.X.X MinGW (SEH) - 64-bit"
   - For Visual Studio: Download "Visual C++ 15 (2017) - 64-bit"

2. Extract to a location like `C:\SFML` or `D:\Libraries\SFML`

3. Set environment variable:
   - Open "Environment Variables" in Windows
   - Add new system variable:
     - Name: `SFML_DIR`
     - Value: Path to SFML (e.g., `C:\SFML\SFML-2.5.1`)
   
4. Add SFML DLLs to PATH or copy them to your project:
   - Copy all `.dll` files from `SFML\bin` to your project's build directory
   - Or add `SFML\bin` to your PATH environment variable

**Option 2: Using vcpkg**

```bash
# Install vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

# Install SFML
vcpkg install sfml:x64-windows

# Integrate with CMake
vcpkg integrate install
```

### Linux (Debian/Ubuntu)

```bash
# Update package list
sudo apt-get update

# Install SFML and dependencies
sudo apt-get install libsfml-dev

# Install CMake if not already installed
sudo apt-get install cmake

# Install build tools
sudo apt-get install build-essential
```

### macOS

```bash
# Using Homebrew (install Homebrew first if needed)
brew install sfml cmake

# If Homebrew is not installed:
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

## Building the Project

### Step 1: Navigate to Project Directory

```bash
cd "D:\CS\Semester 3\Data Structures and Algorithms\Project\DSA_Chess_Engine"
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

**Windows (MinGW):**
```bash
cmake .. -G "MinGW Makefiles"
```

**Windows (Visual Studio):**
```bash
cmake .. -G "Visual Studio 16 2019"
# Or for newer versions:
cmake .. -G "Visual Studio 17 2022"
```

**Linux/macOS:**
```bash
cmake ..
```

**If SFML is not in standard location (Windows):**
```bash
cmake .. -DSFML_DIR="C:/SFML/SFML-2.5.1/lib/cmake/SFML"
```

### Step 4: Build

**Windows (MinGW):**
```bash
mingw32-make
```

**Windows (Visual Studio):**
```bash
cmake --build . --config Release
```

**Linux/macOS:**
```bash
make
```

### Step 5: Run

**Windows:**
```bash
# Main chess GUI
ChessEngine.exe

# Perft testing
PerftTest.exe
```

**Linux/macOS:**
```bash
# Main chess GUI
./ChessEngine

# Perft testing
./PerftTest
```

## Troubleshooting

### Problem: "SFML/Graphics.hpp: No such file or directory"

**Solution:** SFML is not installed or not found by CMake.

**Fix:**
1. Make sure SFML is installed (see instructions above)
2. Set `SFML_DIR` environment variable to point to SFML installation
3. Or specify manually in CMake:
   ```bash
   cmake .. -DSFML_DIR="path/to/sfml/lib/cmake/SFML"
   ```

### Problem: "DLL not found" when running on Windows

**Solution:** SFML DLLs need to be accessible.

**Fix:**
1. Copy DLLs from `SFML\bin` to your build directory
2. Or add `SFML\bin` to system PATH
3. Required DLLs:
   - `sfml-graphics-2.dll`
   - `sfml-window-2.dll`
   - `sfml-system-2.dll`

### Problem: CMake can't find SFML

**Solution:** Point CMake to SFML location.

**Fix:**
```bash
cmake .. -DSFML_DIR="C:/SFML/SFML-2.5.1/lib/cmake/SFML"
```

### Problem: "error: 'Board' was not declared"

**Solution:** Include paths issue or compilation order problem.

**Fix:**
- Make sure you're using CMake to build (not compiling manually)
- Clean build directory: `rm -rf build && mkdir build`
- Try again

### Problem: Build is very slow

**Solution:** Use parallel compilation.

**Fix:**
```bash
# Linux/macOS
make -j4  # Use 4 cores

# Windows (Visual Studio)
cmake --build . --config Release -- /m:4
```

## Quick Build Script

Create a file `build.bat` (Windows) or `build.sh` (Linux/macOS):

**Windows (build.bat):**
```batch
@echo off
echo Building Chess Engine...
if not exist build mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
echo.
echo Build complete!
echo Run ChessEngine.exe to play
pause
```

**Linux/macOS (build.sh):**
```bash
#!/bin/bash
echo "Building Chess Engine..."
mkdir -p build
cd build
cmake ..
make -j4
echo ""
echo "Build complete!"
echo "Run ./ChessEngine to play"
```

Make executable: `chmod +x build.sh`

## Testing the Build

After building successfully:

1. **Test Perft:**
   ```bash
   ./PerftTest
   ```
   Expected output: Should show "PASSED" for depth 1-3 tests

2. **Run GUI:**
   ```bash
   ./ChessEngine
   ```
   Expected: Chess board window appears, pieces are clickable

3. **Quick sanity check:**
   - Can you see the board?
   - Can you click and move pieces?
   - Does undo (U key) work?

## Building for Release (Optimized)

For best performance:

```bash
mkdir build-release
cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

This enables optimizations (~10x faster than debug build).

## Cleaning Build

If you need to start fresh:

```bash
# Remove build directory
rm -rf build

# Or use CMake
cd build
cmake --build . --target clean
```

## IDE Setup

### Visual Studio Code

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft)

2. Configure include paths:
   - Press `Ctrl+Shift+P`
   - Type "C/C++: Edit Configurations"
   - Add SFML include path

3. Build with CMake Tools:
   - Press `Ctrl+Shift+P`
   - Type "CMake: Configure"
   - Then "CMake: Build"

### Visual Studio

1. Open CMakeLists.txt as project
2. Visual Studio will auto-configure
3. Build with `Ctrl+Shift+B`

### CLion

1. Open project folder
2. CLion auto-detects CMakeLists.txt
3. Build with `Ctrl+F9`

## Common Build Commands Reference

```bash
# Configure
cmake ..

# Build
cmake --build .

# Build with optimizations
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Clean
cmake --build . --target clean

# Verbose output
cmake --build . --verbose

# Parallel build (4 cores)
cmake --build . -j4
```

## Next Steps

After successful build:

1. ✅ Run `./PerftTest` to validate move generation
2. ✅ Run `./ChessEngine` to test the GUI
3. ✅ Read `README.md` for usage instructions
4. ✅ Read `docs/API.md` if integrating with other modules

## Getting Help

If you encounter issues:

1. Check error messages carefully
2. Verify SFML installation
3. Check CMake output for clues
4. Try cleaning and rebuilding
5. Refer to SFML documentation: https://www.sfml-dev.org/tutorials/

## Performance Notes

**Debug Build:**
- Perft depth 5: ~30 seconds
- Good for development and debugging

**Release Build:**
- Perft depth 5: ~5 seconds
- Use for testing and demonstration

Always use Release build for performance testing!

---

**Good luck with your build! ♟️**

If you still have issues, feel free to ask for help.

