# AI Agent Prompt: Chess Engine GUI + Move Generation Module

## Project Context
You are implementing the foundational module of a Chess Engine for a university DSA (Data Structures and Algorithms) course project. This is a team project with 4 members:
- **Your role:** GUI + Move Generation (Foundation Layer)
- **Teammate 1:** Search (Alpha-Beta Pruning, Iterative Deepening)
- **Teammate 2:** Search Optimization (Transposition Tables, Move Ordering)
- **Teammate 3:** Evaluation + Testing

Your work is the foundation upon which all other modules depend. You must create clean, well-documented APIs for your teammates.

---

## Technical Requirements

### Board Representation: 0x88 (Mandatory)
You MUST use the 0x88 board representation method:
- 128-element array (16×8) where only the first 8 columns are used
- Allows fast boundary checking: `if (square & 0x88) // off board`
- Efficient for move validation and generation
- Good balance between simplicity and performance

**0x88 Board Layout:**
```
Rank 8: 0x70-0x77 (112-119) | 0x78-0x7F (unused)
Rank 7: 0x60-0x67 (96-103)  | 0x68-0x6F (unused)
Rank 6: 0x50-0x57 (80-87)   | 0x58-0x5F (unused)
Rank 5: 0x40-0x47 (64-71)   | 0x48-0x4F (unused)
Rank 4: 0x30-0x37 (48-55)   | 0x38-0x3F (unused)
Rank 3: 0x20-0x27 (32-39)   | 0x28-0x2F (unused)
Rank 2: 0x10-0x17 (16-23)   | 0x18-0x1F (unused)
Rank 1: 0x00-0x07 (0-7)     | 0x08-0x0F (unused)
        a-h files             (off-board)
```

### Programming Language & Libraries
- **Language:** C++ (C++17 or later)
- **GUI Library:** SFML 2.5+ (Simple and Fast Multimedia Library)
- **Build System:** CMake
- **Coding Standard:** Google C++ Style Guide (with flexibility)

### File Structure
```
chess-engine/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── API.md                    (API documentation for teammates)
│   ├── 0x88_EXPLANATION.md       (Explain 0x88 to instructor)
│   └── COMPLEXITY_ANALYSIS.md    (Time/space complexity)
├── src/
│   ├── main.cpp
│   ├── board/
│   │   ├── board.h
│   │   ├── board.cpp
│   │   ├── move.h
│   │   └── constants.h
│   ├── movegen/
│   │   ├── movegen.h
│   │   ├── movegen.cpp
│   │   ├── pawn_moves.cpp
│   │   ├── knight_moves.cpp
│   │   ├── sliding_moves.cpp
│   │   ├── king_moves.cpp
│   │   └── legal_filter.cpp
│   ├── gui/
│   │   ├── chess_gui.h
│   │   ├── chess_gui.cpp
│   │   └── resource_manager.cpp
│   └── utils/
│       ├── fen.h
│       ├── fen.cpp
│       └── debug.cpp
├── tests/
│   ├── test_movegen.cpp
│   ├── test_board.cpp
│   └── perft_test.cpp
└── assets/
    └── pieces/
        ├── white_pawn.png
        ├── white_knight.png
        └── (other piece images)
```

---

## Implementation Tasks

### Phase 1: Board Representation (Priority: CRITICAL)

#### Task 1.1: Core Board Class
Create a `Board` class with 0x88 representation:

```cpp
class Board {
private:
    int board[128];  // 0x88 board representation
    
    // Game state
    int sideToMove;        // 0 = white, 1 = black
    int castlingRights;    // 4 bits: 0x01=K, 0x02=Q, 0x04=k, 0x08=q
    int enPassantSquare;   // 0x88 square index, -1 if none
    int halfMoveClock;     // For 50-move rule
    int fullMoveNumber;
    
    // Move history for undo
    std::vector<MoveInfo> moveHistory;
    
    // Precomputed tables
    static int knightMoves[8];     // Knight move offsets
    static int bishopDirections[4]; // Bishop directions
    static int rookDirections[4];   // Rook directions
    static int kingMoves[8];        // King move offsets
    
public:
    Board();
    void setStartingPosition();
    void setFEN(const std::string& fen);
    std::string getFEN() const;
    
    // Core interface for teammates
    std::vector<Move> generateLegalMoves();
    bool makeMove(const Move& move);
    void undoMove();
    bool isCheck(int side) const;
    bool isCheckmate();
    bool isStalemate();
    bool isDraw() const;
    
    // Helper functions
    int getPiece(int square) const;
    void setPiece(int square, int piece);
    bool isValidSquare(int square) const;
    
    // Display for debugging
    void print() const;
};
```

**Piece Encoding:**
```cpp
// Piece values (use these constants)
enum Piece {
    EMPTY = 0,
    WHITE_PAWN = 1, WHITE_KNIGHT = 2, WHITE_BISHOP = 3,
    WHITE_ROOK = 4, WHITE_QUEEN = 5, WHITE_KING = 6,
    BLACK_PAWN = -1, BLACK_KNIGHT = -2, BLACK_BISHOP = -3,
    BLACK_ROOK = -4, BLACK_QUEEN = -5, BLACK_KING = -6
};
```

#### Task 1.2: Move Structure
```cpp
struct Move {
    int from;              // 0x88 square
    int to;                // 0x88 square
    int piece;             // Moving piece
    int capturedPiece;     // 0 if no capture
    int promotionPiece;    // 0 if not a promotion
    int flags;             // Special move flags
    
    // For move ordering (used by optimization team)
    int score;
    
    Move() : from(0), to(0), piece(0), capturedPiece(0), 
             promotionPiece(0), flags(0), score(0) {}
    
    std::string toAlgebraic() const;  // e.g., "e2e4"
};

// Move flags
const int FLAG_NONE = 0;
const int FLAG_CASTLING_KINGSIDE = 1;
const int FLAG_CASTLING_QUEENSIDE = 2;
const int FLAG_EN_PASSANT = 4;
const int FLAG_PROMOTION = 8;
const int FLAG_DOUBLE_PAWN_PUSH = 16;
```

#### Task 1.3: 0x88 Helper Functions
```cpp
// CRITICAL: Implement these helper functions
inline bool isValidSquare(int square) {
    return (square & 0x88) == 0;
}

inline int fileOf(int square) {
    return square & 7;  // Get file (0-7)
}

inline int rankOf(int square) {
    return square >> 4;  // Get rank (0-7)
}

inline int makeSquare(int file, int rank) {
    return (rank << 4) | file;
}

// Convert between 0x88 and 0-63 representation
inline int to0x88(int square64) {
    return ((square64 >> 3) << 4) | (square64 & 7);
}

inline int from0x88(int square88) {
    return ((square88 >> 4) << 3) | (square88 & 7);
}

// For algebraic notation
std::string squareToAlgebraic(int square88) {
    int file = fileOf(square88);
    int rank = rankOf(square88);
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}
```

---

### Phase 2: Move Generation (Priority: CRITICAL)

#### Task 2.1: Pseudo-Legal Move Generation
Implement move generation for each piece type:

**Knight Moves (Simplest - Start Here):**
```cpp
// Precomputed knight move offsets
const int KNIGHT_OFFSETS[8] = {-33, -31, -18, -14, 14, 18, 31, 33};

void generateKnightMoves(std::vector<Move>& moves, int from) {
    int piece = board[from];
    
    for (int i = 0; i < 8; i++) {
        int to = from + KNIGHT_OFFSETS[i];
        
        // 0x88 boundary check - this is the magic!
        if (to & 0x88) continue;
        
        int target = board[to];
        
        // Can't capture own pieces
        if ((piece > 0 && target > 0) || (piece < 0 && target < 0)) 
            continue;
        
        moves.push_back(Move(from, to, piece, target));
    }
}
```

**Sliding Pieces (Bishop, Rook, Queen):**
```cpp
// Direction offsets
const int BISHOP_DIRECTIONS[4] = {-17, -15, 15, 17};
const int ROOK_DIRECTIONS[4] = {-16, -1, 1, 16};

void generateSlidingMoves(std::vector<Move>& moves, int from, 
                         const int* directions, int numDirections) {
    int piece = board[from];
    
    for (int i = 0; i < numDirections; i++) {
        int dir = directions[i];
        int to = from + dir;
        
        // Slide in this direction until hitting edge or piece
        while (!(to & 0x88)) {  // While on board
            int target = board[to];
            
            if (target == EMPTY) {
                moves.push_back(Move(from, to, piece, 0));
                to += dir;  // Continue in this direction
            } else {
                // Hit a piece
                if ((piece > 0 && target < 0) || (piece < 0 && target > 0)) {
                    // Can capture opponent's piece
                    moves.push_back(Move(from, to, piece, target));
                }
                break;  // Can't move further in this direction
            }
        }
    }
}
```

**Pawn Moves (Most Complex):**
```cpp
void generatePawnMoves(std::vector<Move>& moves, int from) {
    int piece = board[from];
    int direction = (piece > 0) ? 16 : -16;  // White up, black down
    int startRank = (piece > 0) ? 1 : 6;
    int promotionRank = (piece > 0) ? 7 : 0;
    
    // Single push
    int to = from + direction;
    if (!(to & 0x88) && board[to] == EMPTY) {
        if (rankOf(to) == promotionRank) {
            // Promotion
            addPromotionMoves(moves, from, to, piece);
        } else {
            moves.push_back(Move(from, to, piece, 0));
        }
        
        // Double push from starting position
        if (rankOf(from) == startRank) {
            int doubleTo = from + 2 * direction;
            if (board[doubleTo] == EMPTY) {
                Move m(from, doubleTo, piece, 0);
                m.flags = FLAG_DOUBLE_PAWN_PUSH;
                moves.push_back(m);
            }
        }
    }
    
    // Captures (including en passant)
    int captureOffsets[2] = {direction - 1, direction + 1};
    for (int i = 0; i < 2; i++) {
        int to = from + captureOffsets[i];
        
        if (to & 0x88) continue;  // Off board
        
        int target = board[to];
        
        // Normal capture
        if ((piece > 0 && target < 0) || (piece < 0 && target > 0)) {
            if (rankOf(to) == promotionRank) {
                addPromotionMoves(moves, from, to, piece, target);
            } else {
                moves.push_back(Move(from, to, piece, target));
            }
        }
        
        // En passant
        if (to == enPassantSquare) {
            Move m(from, to, piece, (piece > 0) ? BLACK_PAWN : WHITE_PAWN);
            m.flags = FLAG_EN_PASSANT;
            moves.push_back(m);
        }
    }
}
```

**King Moves + Castling:**
```cpp
void generateKingMoves(std::vector<Move>& moves, int from) {
    int piece = board[from];
    const int KING_OFFSETS[8] = {-17, -16, -15, -1, 1, 15, 16, 17};
    
    // Normal king moves
    for (int i = 0; i < 8; i++) {
        int to = from + KING_OFFSETS[i];
        
        if (to & 0x88) continue;
        
        int target = board[to];
        if ((piece > 0 && target > 0) || (piece < 0 && target < 0)) 
            continue;
        
        moves.push_back(Move(from, to, piece, target));
    }
    
    // Castling
    if (!isCheck(sideToMove)) {
        generateCastlingMoves(moves, from);
    }
}

void generateCastlingMoves(std::vector<Move>& moves, int kingSquare) {
    // Implement kingside and queenside castling logic
    // Check: castling rights, no pieces between, no squares under attack
}
```

#### Task 2.2: Legal Move Filtering
```cpp
std::vector<Move> Board::generateLegalMoves() {
    std::vector<Move> pseudoLegal = generatePseudoLegalMoves();
    std::vector<Move> legalMoves;
    
    for (const Move& move : pseudoLegal) {
        makeMove(move);
        
        // If this move doesn't leave our king in check, it's legal
        if (!isCheck(1 - sideToMove)) {  // Check opponent's perspective
            legalMoves.push_back(move);
        }
        
        undoMove();
    }
    
    return legalMoves;
}

bool Board::isCheck(int side) const {
    // Find the king
    int king = (side == 0) ? WHITE_KING : BLACK_KING;
    int kingSquare = -1;
    
    for (int sq = 0; sq < 128; sq++) {
        if (!(sq & 0x88) && board[sq] == king) {
            kingSquare = sq;
            break;
        }
    }
    
    // Check if any opponent piece attacks the king
    return isSquareAttacked(kingSquare, 1 - side);
}

bool Board::isSquareAttacked(int square, int attackingSide) const {
    // Check knight attacks
    // Check bishop/queen attacks (diagonal rays)
    // Check rook/queen attacks (straight rays)
    // Check pawn attacks
    // Check king attacks
    // Use 0x88 boundary checking throughout
}
```

#### Task 2.3: Make/Undo Move
```cpp
struct MoveInfo {
    Move move;
    int capturedPiece;
    int castlingRights;
    int enPassantSquare;
    int halfMoveClock;
};

bool Board::makeMove(const Move& move) {
    // Save state for undo
    MoveInfo info;
    info.move = move;
    info.capturedPiece = move.capturedPiece;
    info.castlingRights = castlingRights;
    info.enPassantSquare = enPassantSquare;
    info.halfMoveClock = halfMoveClock;
    moveHistory.push_back(info);
    
    // Move the piece
    board[move.to] = board[move.from];
    board[move.from] = EMPTY;
    
    // Handle special moves
    if (move.flags & FLAG_CASTLING_KINGSIDE) {
        // Move the rook
    }
    if (move.flags & FLAG_CASTLING_QUEENSIDE) {
        // Move the rook
    }
    if (move.flags & FLAG_EN_PASSANT) {
        // Remove the captured pawn
    }
    if (move.flags & FLAG_PROMOTION) {
        board[move.to] = move.promotionPiece;
    }
    
    // Update game state
    enPassantSquare = (move.flags & FLAG_DOUBLE_PAWN_PUSH) 
                      ? (move.from + move.to) / 2 : -1;
    
    // Update castling rights
    updateCastlingRights(move);
    
    // Update move counters
    halfMoveClock++;
    if (move.piece == WHITE_PAWN || move.piece == BLACK_PAWN 
        || move.capturedPiece != 0) {
        halfMoveClock = 0;
    }
    
    if (sideToMove == 1) fullMoveNumber++;
    sideToMove = 1 - sideToMove;
    
    return true;
}

void Board::undoMove() {
    if (moveHistory.empty()) return;
    
    MoveInfo info = moveHistory.back();
    moveHistory.pop_back();
    
    // Restore side to move
    sideToMove = 1 - sideToMove;
    if (sideToMove == 1) fullMoveNumber--;
    
    // Restore the piece
    board[info.move.from] = board[info.move.to];
    board[info.move.to] = info.capturedPiece;
    
    // Handle special moves (reverse)
    // Restore game state
    castlingRights = info.castlingRights;
    enPassantSquare = info.enPassantSquare;
    halfMoveClock = info.halfMoveClock;
}
```

---

### Phase 3: GUI Implementation (Priority: HIGH)

#### Task 3.1: Basic GUI Setup
```cpp
class ChessGUI {
private:
    Board& board;
    sf::RenderWindow window;
    
    // Visual settings
    static const int SQUARE_SIZE = 80;
    static const int BOARD_SIZE = SQUARE_SIZE * 8;
    
    // Piece textures
    std::map<int, sf::Texture> pieceTextures;
    
    // Interaction state
    int selectedSquare;
    std::vector<Move> selectedMoves;
    bool isDragging;
    sf::Vector2f dragOffset;
    
public:
    ChessGUI(Board& b);
    void run();
    
private:
    void loadTextures();
    void handleEvents();
    void render();
    
    // Coordinate conversion
    int screenToSquare(int x, int y);
    sf::Vector2f squareToScreen(int square88);
    
    // Drawing
    void drawBoard();
    void drawPieces();
    void drawLegalMoves();
    void highlightSquare(int square88, sf::Color color);
};
```

#### Task 3.2: Event Handling
```cpp
void ChessGUI::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                int square = screenToSquare(event.mouseButton.x, 
                                           event.mouseButton.y);
                
                if (selectedSquare == -1) {
                    // Select a piece
                    int piece = board.getPiece(square);
                    if (piece != EMPTY && 
                        ((board.getSideToMove() == 0 && piece > 0) ||
                         (board.getSideToMove() == 1 && piece < 0))) {
                        selectedSquare = square;
                        selectedMoves = board.getLegalMovesFrom(square);
                    }
                } else {
                    // Try to move
                    Move move = findMove(selectedSquare, square);
                    if (move.from != 0) {
                        board.makeMove(move);
                    }
                    selectedSquare = -1;
                    selectedMoves.clear();
                }
            }
        }
    }
}
```

#### Task 3.3: Rendering
```cpp
void ChessGUI::render() {
    window.clear();
    
    drawBoard();
    
    // Highlight selected square
    if (selectedSquare != -1) {
        highlightSquare(selectedSquare, sf::Color(255, 255, 0, 100));
    }
    
    drawLegalMoves();
    drawPieces();
    
    // Draw additional info (captured pieces, move history, etc.)
    
    window.display();
}

void ChessGUI::drawBoard() {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
            square.setPosition(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE);
            
            if ((rank + file) % 2 == 0) {
                square.setFillColor(sf::Color(240, 217, 181)); // Light square
            } else {
                square.setFillColor(sf::Color(181, 136, 99));  // Dark square
            }
            
            window.draw(square);
        }
    }
}

void ChessGUI::drawPieces() {
    for (int square88 = 0; square88 < 128; square88++) {
        if (square88 & 0x88) continue;  // Skip off-board squares
        
        int piece = board.getPiece(square88);
        if (piece == EMPTY) continue;
        
        sf::Sprite sprite(pieceTextures[piece]);
        sf::Vector2f pos = squareToScreen(square88);
        sprite.setPosition(pos);
        window.draw(sprite);
    }
}
```

#### Task 3.4: Coordinate Conversion (CRITICAL for 0x88)
```cpp
int ChessGUI::screenToSquare(int x, int y) {
    int file = x / SQUARE_SIZE;
    int rank = 7 - (y / SQUARE_SIZE);
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        return -1;  // Invalid
    }
    
    return makeSquare(file, rank);  // Convert to 0x88
}

sf::Vector2f ChessGUI::squareToScreen(int square88) {
    int file = fileOf(square88);
    int rank = rankOf(square88);
    
    float x = file * SQUARE_SIZE;
    float y = (7 - rank) * SQUARE_SIZE;
    
    return sf::Vector2f(x, y);
}
```

---

### Phase 4: Testing & Documentation

#### Task 4.1: Perft Testing (CRITICAL for DSA grade)
Implement perft (performance test) - counts nodes at each depth:

```cpp
uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1;
    
    uint64_t nodes = 0;
    std::vector<Move> moves = board.generateLegalMoves();
    
    for (const Move& move : moves) {
        board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.undoMove();
    }
    
    return nodes;
}

// Expected results for starting position:
// Depth 1: 20 nodes
// Depth 2: 400 nodes
// Depth 3: 8,902 nodes
// Depth 4: 197,281 nodes
// Depth 5: 4,865,609 nodes
```

#### Task 4.2: API Documentation
Create `docs/API.md` with:
- Function signatures
- Parameter descriptions
- Return value descriptions
- Time complexity analysis
- Example usage code
- Integration guide for teammates

#### Task 4.3: 0x88 Explanation Document
Create `docs/0x88_EXPLANATION.md` explaining:
- Why 0x88 was chosen
- How boundary checking works
- Visual diagrams
- Comparison with other representations
- DSA concepts demonstrated

---

## Deliverables Checklist

### Week 1
- [ ] Project structure set up
- [ ] CMakeLists.txt configured
- [ ] Board class with 0x88 representation
- [ ] Basic FEN parsing
- [ ] Helper functions (coordinate conversion, etc.)

### Week 2
- [ ] All piece move generation implemented
- [ ] Legal move filtering working
- [ ] makeMove/undoMove functional
- [ ] Check/checkmate/stalemate detection
- [ ] Perft test passing for depth 1-3

### Week 3
- [ ] GUI displaying board and pieces
- [ ] User can select and move pieces
- [ ] Legal moves highlighted
- [ ] Special moves working (castling, en passant, promotion)
- [ ] Basic move history display

### Week 4
- [ ] API documentation complete
- [ ] Code comments thorough
- [ ] Integration testing with teammates
- [ ] Performance optimization
- [ ] Final bug fixes

---

## Code Quality Requirements

### Documentation
- Every function must have a header comment explaining:
  - Purpose
  - Parameters
  - Return value
  - Time complexity
  - Space complexity (if relevant)

### Naming Conventions
- Classes: PascalCase (e.g., `ChessBoard`)
- Functions: camelCase (e.g., `generateMoves`)
- Constants: UPPER_SNAKE_CASE (e.g., `SQUARE_SIZE`)
- Variables: camelCase (e.g., `selectedSquare`)

### Error Handling
- Validate all inputs
- Use assertions for internal consistency checks
- Provide meaningful error messages

### Performance
- No memory leaks
- Move generation < 1ms per position
- GUI runs at 60 FPS
- Efficient use of 0x88 boundary checking

---

## Success Criteria

### Functional
✅ All pieces move according to chess rules  
✅ Special moves work correctly  
✅ Check/checkmate/stalemate detected accurately  
✅ GUI is responsive and intuitive  
✅ Perft tests pass for depth 5  
✅ No crashes or undefined behavior  

### DSA Requirements
✅ Clear demonstration of data structure choice (0x88)  
✅ Algorithm complexity analysis documented  
✅ Efficient implementation (profiling data)  
✅ Clean API design for modularity  

### Team Integration
✅ API documentation clear and complete  
✅ Example code provided  
✅ Integration support available  
✅ No blocking issues for teammates  

---

## Important Notes

1. **Priority Order:**
   - Board representation > Move generation > Legal filtering > GUI > Polish

2. **0x88 Advantages to Emphasize:**
   - Fast boundary checking: `if (square & 0x88)`
   - Simpler than bitboards for beginners
   - Still professional-grade (used in many engines)
   - Good balance of performance and readability

3. **Common Pitfalls to Avoid:**
   - Forgetting to update game state (castling rights, en passant)
   - Not handling move undo correctly
   - Off-by-one errors in 0x88 conversion
   - Memory leaks in move generation

4. **DSA Concepts to Highlight:**
   - Array-based data structure (0x88)
   - Graph traversal (move generation)
   - Backtracking (legal move filtering)
   - Stack (move history)
   - Algorithmic complexity analysis

---

## Final Checklist Before Submission

- [ ] All code compiles without warnings
- [ ] Perft tests pass
- [ ] GUI demo works flawlessly
- [ ] API.md complete
- [ ] 0x88_EXPLANATION.md written
- [ ] COMPLEXITY_ANALYSIS.md complete
- [ ] README.md with build instructions
- [ ] Code is well-commented
- [ ] No hardcoded values (use constants)
- [ ] Teammates can integrate successfully

---

## Output Format

For each implementation task:
1. Write complete, production-ready code
2. Include comprehensive comments
3. Add error handling
4. Provide usage examples
5. Document time/space complexity

Start with Phase 1, Task 1.1 and proceed sequentially. Ask for clarification if any requirement is unclear.

**Begin implementation now.**