# Chess Engine API Documentation

## For Teammates: Search, Optimization, and Evaluation Modules

This document provides a complete API reference for integrating your modules with the Move Generation and GUI foundation layer.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Core Classes](#core-classes)
3. [Board Representation](#board-representation)
4. [Move Generation API](#move-generation-api)
5. [Integration Examples](#integration-examples)
6. [Performance Considerations](#performance-considerations)
7. [Testing](#testing)

---

## Quick Start

### Basic Usage

```cpp
#include "Board.h"

// Create and initialize board
Board board;
board.setStartingPosition();

// Generate all legal moves
std::vector<Move> moves = board.generateLegalMoves();

// Make a move
Move move = moves[0];
board.makeMove(move);

// Undo the move
board.undoMove();

// Check game state
bool inCheck = board.isCheck(0);  // 0 = white, 1 = black
bool isCheckmate = board.isCheckmate();
```

---

## Core Classes

### Board Class

The `Board` class is your main interface for all board operations.

**Location:** `include/Board.h`, `src/Board.cpp`

**Key Features:**
- 0x88 board representation (128-element array)
- Complete move generation
- Make/undo move functionality
- Game state queries (check, checkmate, stalemate)
- FEN import/export

### Move Structure

```cpp
struct Move {
    int from;              // Source square (0x88)
    int to;                // Destination square (0x88)
    int piece;             // Moving piece
    int capturedPiece;     // Captured piece (0 if none)
    int promotionPiece;    // Promotion piece (0 if not promotion)
    int flags;             // Special move flags
    int score;             // Move ordering score (YOU SET THIS!)
};
```

**Important:** The `score` field is for your move ordering optimization!

---

## Board Representation

### 0x88 Layout

We use 0x88 board representation:

```
Rank 8: 0x70-0x77 (112-119)
Rank 7: 0x60-0x67 (96-103)
Rank 6: 0x50-0x57 (80-87)
Rank 5: 0x40-0x47 (64-71)
Rank 4: 0x30-0x37 (48-55)
Rank 3: 0x20-0x27 (32-39)
Rank 2: 0x10-0x17 (16-23)
Rank 1: 0x00-0x07 (0-7)
        a-h files
```

### Helper Functions

```cpp
// Check if square is valid
bool isValid0x88(int square);         // Time: O(1)

// Convert coordinates
int fileOf(int square);                // Get file (0-7)
int rankOf(int square);                // Get rank (0-7)
int makeSquare(int file, int rank);   // Create square

// Convert notations
std::string squareToAlgebraic(int square88);  // e.g., "e4"
int algebraicToSquare(const std::string& alg);
```

### Piece Encoding

```cpp
enum Piece {
    EMPTY = 0,
    WHITE_PAWN = 1, WHITE_KNIGHT = 2, WHITE_BISHOP = 3,
    WHITE_ROOK = 4, WHITE_QUEEN = 5, WHITE_KING = 6,
    BLACK_PAWN = -1, BLACK_KNIGHT = -2, BLACK_BISHOP = -3,
    BLACK_ROOK = -4, BLACK_QUEEN = -5, BLACK_KING = -6
};
```

**Rule:** Positive = White, Negative = Black, Zero = Empty

---

## Move Generation API

### Primary Functions

#### `std::vector<Move> generateLegalMoves()`

Generates all legal moves for the current position.

**Time Complexity:** O(n×m) where n = number of pieces, m = average moves per piece  
**Space Complexity:** O(k) where k = number of legal moves  
**Typical Return:** 20-50 moves in middlegame

```cpp
Board board;
board.setStartingPosition();

std::vector<Move> moves = board.generateLegalMoves();
// Returns 20 moves from starting position

for (const Move& move : moves) {
    std::cout << move.toAlgebraic() << "\n";
}
```

**Usage in Search:**
```cpp
int alphaBeta(Board& board, int depth, int alpha, int beta) {
    if (depth == 0) return evaluate(board);
    
    std::vector<Move> moves = board.generateLegalMoves();
    
    // TODO (Move Ordering Team): Sort moves here!
    // std::sort(moves.begin(), moves.end(), compareByScore);
    
    for (Move& move : moves) {
        board.makeMove(move);
        int score = -alphaBeta(board, depth - 1, -beta, -alpha);
        board.undoMove();
        
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    
    return alpha;
}
```

#### `bool makeMove(const Move& move)`

Makes a move on the board and updates all game state.

**Time Complexity:** O(1)  
**Returns:** true if move was successful

**Updates:**
- Board position
- Side to move
- Castling rights
- En passant square
- Half-move clock (50-move rule)
- Full move number

**Important:** This function handles ALL special moves:
- Castling (kingside and queenside)
- En passant
- Pawn promotion
- Regular captures

```cpp
Move move = moves[0];
bool success = board.makeMove(move);

// Board state is now updated
// You can continue searching or evaluating
```

#### `void undoMove()`

Undoes the last move, restoring all previous game state.

**Time Complexity:** O(1)

**Critical for Search:** Make/undo moves instead of copying the board!

```cpp
// Efficient search pattern
board.makeMove(move);
int score = -search(board, depth - 1);
board.undoMove();  // Restore previous state

// DON'T do this (too slow):
Board copy = board;  // Expensive copy!
copy.makeMove(move);
int score = -search(copy, depth - 1);
```

### Game State Queries

#### `bool isCheck(int side) const`

Checks if the given side is in check.

**Time Complexity:** O(n) where n = number of opponent pieces  
**Parameters:** `side` - 0 for white, 1 for black

```cpp
if (board.isCheck(0)) {
    std::cout << "White is in check!\n";
}

// Use in evaluation
int checkBonus = board.isCheck(1 - sideToMove) ? 50 : 0;
```

#### `bool isCheckmate()`

Checks if current side is checkmated.

**Time Complexity:** O(n×m) - generates all legal moves  
**Returns:** true if checkmate

```cpp
if (board.isCheckmate()) {
    return -MATE_SCORE;  // Game over
}
```

#### `bool isStalemate()`

Checks if position is stalemate.

**Time Complexity:** O(n×m)  
**Returns:** true if stalemate

```cpp
if (board.isStalemate()) {
    return 0;  // Draw
}
```

#### `bool isDraw() const`

Checks for draw conditions (50-move rule, insufficient material).

**Time Complexity:** O(n) where n = number of pieces  
**Returns:** true if position is drawn

```cpp
if (board.isDraw()) {
    return 0;  // Draw
}
```

### Utility Functions

#### `int getPiece(int square) const`

Gets the piece at a given square.

**Time Complexity:** O(1)

```cpp
int piece = board.getPiece(0x34);  // e4 in 0x88
if (piece == WHITE_PAWN) {
    // Central pawn bonus
}
```

#### `int getSideToMove() const`

Returns the side to move (0 = white, 1 = black).

```cpp
int side = board.getSideToMove();
```

#### `std::string getFEN() const`

Gets current position as FEN string.

**Time Complexity:** O(1)

```cpp
std::string fen = board.getFEN();
// Store in transposition table key
```

#### `void setFEN(const std::string& fen)`

Sets position from FEN string.

```cpp
board.setFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
```

---

## Integration Examples

### For Search Team (Alpha-Beta Pruning)

```cpp
#include "Board.h"

const int MATE_SCORE = 100000;

int alphaBeta(Board& board, int depth, int alpha, int beta) {
    // Check for terminal conditions
    if (board.isCheckmate()) {
        return -MATE_SCORE + (maxDepth - depth);
    }
    if (board.isStalemate() || board.isDraw()) {
        return 0;
    }
    
    if (depth == 0) {
        return evaluate(board);  // Call evaluation module
    }
    
    // Get legal moves
    std::vector<Move> moves = board.generateLegalMoves();
    
    // Alpha-beta search
    for (Move& move : moves) {
        board.makeMove(move);
        int score = -alphaBeta(board, depth - 1, -beta, -alpha);
        board.undoMove();
        
        if (score >= beta) {
            return beta;  // Beta cutoff
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}
```

### For Optimization Team (Transposition Tables)

```cpp
#include "Board.h"
#include <unordered_map>

struct TTEntry {
    int depth;
    int score;
    int flag;  // EXACT, LOWER_BOUND, UPPER_BOUND
    Move bestMove;
};

std::unordered_map<std::string, TTEntry> transpositionTable;

int searchWithTT(Board& board, int depth, int alpha, int beta) {
    // Probe transposition table
    std::string key = board.getFEN();
    
    if (transpositionTable.count(key)) {
        TTEntry entry = transpositionTable[key];
        if (entry.depth >= depth) {
            // Use stored value
            return entry.score;
        }
    }
    
    // Search...
    int score = alphaBeta(board, depth, alpha, beta);
    
    // Store in transposition table
    TTEntry newEntry;
    newEntry.depth = depth;
    newEntry.score = score;
    transpositionTable[key] = newEntry;
    
    return score;
}
```

### For Optimization Team (Move Ordering)

```cpp
#include "Board.h"

void orderMoves(std::vector<Move>& moves, Board& board) {
    for (Move& move : moves) {
        int score = 0;
        
        // MVV-LVA: Most Valuable Victim - Least Valuable Attacker
        if (move.capturedPiece != 0) {
            score += 10 * std::abs(move.capturedPiece) - std::abs(move.piece);
        }
        
        // Promotion
        if (move.flags & FLAG_PROMOTION) {
            score += 8;
        }
        
        // Checks
        board.makeMove(move);
        if (board.isCheck(1 - board.getSideToMove())) {
            score += 5;
        }
        board.undoMove();
        
        move.score = score;
    }
    
    // Sort moves by score (descending)
    std::sort(moves.begin(), moves.end(), 
              [](const Move& a, const Move& b) {
                  return a.score > b.score;
              });
}
```

### For Evaluation Team

```cpp
#include "Board.h"

int evaluate(Board& board) {
    int score = 0;
    
    // Material evaluation
    for (int sq = 0; sq < 128; sq++) {
        if (sq & 0x88) continue;  // Skip off-board
        
        int piece = board.getPiece(sq);
        if (piece == EMPTY) continue;
        
        int value = getPieceValue(piece);
        int positional = getPositionalBonus(piece, sq);
        
        if (piece > 0) {
            score += value + positional;
        } else {
            score -= value + positional;
        }
    }
    
    // Bonus for side to move
    if (board.getSideToMove() == 1) {
        score = -score;
    }
    
    return score;
}

int getPieceValue(int piece) {
    switch (std::abs(piece)) {
        case 1: return 100;   // Pawn
        case 2: return 320;   // Knight
        case 3: return 330;   // Bishop
        case 4: return 500;   // Rook
        case 5: return 900;   // Queen
        case 6: return 20000; // King
        default: return 0;
    }
}
```

---

## Performance Considerations

### DO ✅

1. **Use make/undo instead of board copying**
   ```cpp
   board.makeMove(move);
   search(board, depth - 1);
   board.undoMove();
   ```

2. **Cache legal moves when possible**
   ```cpp
   std::vector<Move> moves = board.generateLegalMoves();
   // Use moves multiple times
   ```

3. **Use move ordering to improve alpha-beta**
   ```cpp
   std::sort(moves.begin(), moves.end(), compareByScore);
   ```

### DON'T ❌

1. **Don't copy the board unnecessarily**
   ```cpp
   Board copy = board;  // Expensive!
   ```

2. **Don't regenerate moves multiple times**
   ```cpp
   for (int i = 0; i < board.generateLegalMoves().size(); i++) {
       // BAD: Regenerates moves every iteration!
   }
   ```

3. **Don't forget to undo moves**
   ```cpp
   board.makeMove(move);
   search(board, depth - 1);
   // BUG: Forgot undoMove()!
   ```

### Performance Benchmarks

- Move generation: ~0.5-1ms per position
- Make/undo move: ~0.1μs (100 nanoseconds)
- Check detection: ~0.2-0.5ms
- Perft depth 5: ~5-10 seconds (optimized build)

---

## Testing

### Perft Testing

Validate your integration with perft tests:

```bash
./PerftTest
```

Expected results for starting position:
- Depth 1: 20 nodes
- Depth 2: 400 nodes
- Depth 3: 8,902 nodes
- Depth 4: 197,281 nodes
- Depth 5: 4,865,609 nodes

### Unit Tests

```cpp
void testMakeUndo() {
    Board board;
    board.setStartingPosition();
    
    std::string originalFEN = board.getFEN();
    std::vector<Move> moves = board.generateLegalMoves();
    
    for (const Move& move : moves) {
        board.makeMove(move);
        board.undoMove();
        
        assert(board.getFEN() == originalFEN);
    }
}
```

---

## Common Issues & Solutions

### Issue: "My search is too slow"

**Solution:** Use make/undo instead of copying boards. Implement move ordering.

### Issue: "Getting different evaluation for same position"

**Solution:** Check if you're properly handling side to move. Evaluations should be from current side's perspective.

### Issue: "Transposition table not working well"

**Solution:** Use FEN as key, or implement Zobrist hashing for better performance.

### Issue: "Some moves cause crashes"

**Solution:** Make sure you're iterating over a copy of moves vector if you modify the board during iteration.

---

## Contact & Support

For questions or issues:
1. Check this documentation
2. Run perft tests to validate move generation
3. Use `board.print()` to debug positions
4. Contact the Move Generation team member

---

## Appendix: Complete API Reference

### Board Class Methods

| Method | Time | Description |
|--------|------|-------------|
| `generateLegalMoves()` | O(n×m) | Generate all legal moves |
| `makeMove(move)` | O(1) | Make a move |
| `undoMove()` | O(1) | Undo last move |
| `isCheck(side)` | O(n) | Check if side in check |
| `isCheckmate()` | O(n×m) | Check for checkmate |
| `isStalemate()` | O(n×m) | Check for stalemate |
| `isDraw()` | O(n) | Check for draw |
| `getPiece(square)` | O(1) | Get piece at square |
| `getFEN()` | O(1) | Get FEN string |
| `setFEN(fen)` | O(1) | Set position from FEN |

### Move Flags

```cpp
FLAG_NONE = 0
FLAG_CASTLING_KINGSIDE = 1
FLAG_CASTLING_QUEENSIDE = 2
FLAG_EN_PASSANT = 4
FLAG_PROMOTION = 8
FLAG_DOUBLE_PAWN_PUSH = 16
```

---

**Good luck with your modules! 🚀**

