# Quick Start Guide - Chess Engine

Get up and running in 5 minutes! ⚡

## 🚀 Fastest Path to Running

### Step 1: Install SFML (Required)

**Windows:**
```bash
# Download from https://www.sfml-dev.org/download.php
# Extract to C:\SFML
# Set environment variable: SFML_DIR = C:\SFML\SFML-2.5.1
```

**Linux:**
```bash
sudo apt-get install libsfml-dev
```

**macOS:**
```bash
brew install sfml
```

### Step 2: Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Step 3: Run

```bash
# Main GUI
./ChessEngine          # Linux/Mac
ChessEngine.exe        # Windows

# Test move generation
./PerftTest
```

## 🎮 Basic Controls

- **Click** piece to select
- **Click** destination to move
- **Drag** pieces to move
- **U** = Undo move
- **R** = Reset board

## ✅ Verify Installation

Run perft test:
```bash
./PerftTest
```

Expected: All tests show "PASSED" ✓

## 📖 Learn More

- **Build Issues?** See [BUILDING.md](BUILDING.md)
- **API Reference?** See [docs/API.md](docs/API.md)
- **How it Works?** See [docs/0x88_EXPLANATION.md](docs/0x88_EXPLANATION.md)

## 🐛 Troubleshooting

**"SFML not found"**
→ Set SFML_DIR environment variable

**"DLL not found" (Windows)**
→ Copy SFML DLLs from SFML\bin to build folder

**Build fails**
→ Check [BUILDING.md](BUILDING.md) for detailed steps

## 🤝 For Teammates

### Integrating Your Module

1. Read [docs/API.md](docs/API.md)
2. See example in `src/Engine.cpp`
3. Use these functions:
   ```cpp
   board.generateLegalMoves()  // Get moves
   board.makeMove(move)        // Make move
   board.undoMove()            // Undo move
   ```

### Testing Your Code

```cpp
#include "Board.h"

int main() {
    Board board;
    board.setStartingPosition();
    
    // Your code here
    auto moves = board.generateLegalMoves();
    
    return 0;
}
```

## 🎯 What's Included

✅ Complete move generation  
✅ GUI with drag-and-drop  
✅ Legal move highlighting  
✅ Check/checkmate detection  
✅ Undo functionality  
✅ Perft validation  
✅ Full documentation  

## 📞 Need Help?

1. Check documentation in `docs/`
2. Run `./PerftTest` to validate setup
3. Review example code in `src/Engine.cpp`

---

**Ready to play chess!** ♟️

For complete details, see [README.md](README.md) and [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md).


