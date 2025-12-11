#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
#include <cstdint>
#include <random>

/**
 * Piece encoding constants
 * Positive values = White pieces
 * Negative values = Black pieces
 * Zero = Empty square
 */
enum Piece
{
    EMPTY = 0,
    WHITE_PAWN = 1,
    WHITE_KNIGHT = 2,
    WHITE_BISHOP = 3,
    WHITE_ROOK = 4,
    WHITE_QUEEN = 5,
    WHITE_KING = 6,
    BLACK_PAWN = -1,
    BLACK_KNIGHT = -2,
    BLACK_BISHOP = -3,
    BLACK_ROOK = -4,
    BLACK_QUEEN = -5,
    BLACK_KING = -6
};

/**
 * Move flags for special moves
 */
const int FLAG_NONE = 0;
const int FLAG_CASTLING_KINGSIDE = 1;
const int FLAG_CASTLING_QUEENSIDE = 2;
const int FLAG_EN_PASSANT = 4;
const int FLAG_PROMOTION = 8;
const int FLAG_DOUBLE_PAWN_PUSH = 16;

/**
 * Castling rights constants
 */
const int CASTLING_WHITE_KINGSIDE = 0x01;
const int CASTLING_WHITE_QUEENSIDE = 0x02;
const int CASTLING_BLACK_KINGSIDE = 0x04;
const int CASTLING_BLACK_QUEENSIDE = 0x08;

// seed
inline uint64_t seed = 42;
/**
 * Move structure representing a chess move
 * Uses 0x88 square representation
 */
struct Move
{
    int from;           // Source square (0x88)
    int to;             // Destination square (0x88)
    int piece;          // Moving piece
    int capturedPiece;  // Captured piece (0 if none)
    int promotionPiece; // Promotion piece (0 if not promotion)
    int flags;          // Special move flags
    int score;          // Move ordering score (for search optimization)

    Move() : from(0), to(0), piece(0), capturedPiece(0),
             promotionPiece(0), flags(0), score(0) {}

    Move(int f, int t, int p, int cap = 0)
        : from(f), to(t), piece(p), capturedPiece(cap),
          promotionPiece(0), flags(FLAG_NONE), score(0) {}

    /**
     * Convert move to algebraic notation (e.g., "e2e4")
     * Time Complexity: O(1)
     */
    std::string toAlgebraic() const;

    /**
     * Convert move to UCI format
     * Time Complexity: O(1)
     */
    std::string toUCI() const;
};

inline bool operator==(const Move &moveA, const Move &moveB)
{
    return moveA.from == moveB.from && moveA.to == moveB.to && moveA.piece == moveB.piece && moveA.capturedPiece == moveB.capturedPiece && moveA.promotionPiece == moveB.promotionPiece && moveA.flags == moveB.flags;
}

/**
 * Structure to store move history for undo functionality
 */
struct MoveInfo
{
    Move move;
    int capturedPiece;
    int castlingRights;
    int enPassantSquare;
    int halfMoveClock;

    MoveInfo() : capturedPiece(0), castlingRights(0),
                 enPassantSquare(-1), halfMoveClock(0) {}
};

/**
 * 0x88 Board Representation
 * -------------------------
 * Uses a 128-element array (16×8) where only the first 8 columns are used.
 * This allows fast boundary checking: if (square & 0x88) // off board
 *
 * Board Layout:
 * Rank 8: 0x70-0x77 (112-119) | 0x78-0x7F (unused)
 * Rank 7: 0x60-0x67 (96-103)  | 0x68-0x6F (unused)
 * Rank 6: 0x50-0x57 (80-87)   | 0x58-0x5F (unused)
 * Rank 5: 0x40-0x47 (64-71)   | 0x48-0x4F (unused)
 * Rank 4: 0x30-0x37 (48-55)   | 0x38-0x3F (unused)
 * Rank 3: 0x20-0x27 (32-39)   | 0x28-0x2F (unused)
 * Rank 2: 0x10-0x17 (16-23)   | 0x18-0x1F (unused)
 * Rank 1: 0x00-0x07 (0-7)     | 0x08-0x0F (unused)
 *         a-h files             (off-board)
 */
class Board
{
private:
    int board[128]; // 0x88 board representation

    // Game state
    int sideToMove;      // 0 = white, 1 = black
    int castlingRights;  // 4 bits: K=0x01, Q=0x02, k=0x04, q=0x08
    int enPassantSquare; // 0x88 square index, -1 if none
    int halfMoveClock;   // For 50-move rule
    int fullMoveNumber;  // Full move counter

    // Move history for undo
    std::vector<MoveInfo> moveHistory;

    // Precomputed move direction offsets
    static const int KNIGHT_OFFSETS[8];
    static const int BISHOP_DIRECTIONS[4];
    static const int ROOK_DIRECTIONS[4];
    static const int KING_OFFSETS[8];

    // Helper functions
    uint64_t rand64(uint64_t &state);

public:
    /**
     * Constructor - creates empty board
     * Time Complexity: O(1)
     */
    Board();
    uint64_t hash; // Current Board hash value
    uint64_t zobristPieceSquare[12][128];
    uint64_t zobristCastling[16];
    uint64_t zobristPassant[8];
    uint64_t zobristSide;

    /**
     * Sets up the standard starting chess position
     * Time Complexity: O(1)
     */
    void setStartingPosition();

    /**
     * Sets board position from FEN string
     * Time Complexity: O(1)
     * @param fen - Forsyth-Edwards Notation string
     * @return true if FEN is valid
     */
    bool setFEN(const std::string &fen);

    /**
     * Gets current position as FEN string
     * Time Complexity: O(1)
     * @return FEN string representation
     */
    std::string getFEN() const;

    // ==================== HASHING FUNCTIONS ========================

    int pieceToZobristNumbering(int piece);
    void initZobristArrays();
    void setHash();
    inline void hashUpdate(Move move);
    void undoHashUpdate(Move move);
    uint64_t moveHash(uint64_t currentHash, Move move);

    // ==================== CORE API FOR TEAMMATES ====================

    /**
     * Generates all legal moves for current position
     * Time Complexity: O(n*m) where n = number of pieces, m = avg moves per piece
     * Space Complexity: O(k) where k = number of legal moves
     * @return Vector of all legal moves
     */
    std::vector<Move> generateLegalMoves();
    std::vector<Move> generatePseudoLegalMoves() const;

    /**
     * Generates legal moves from a specific square
     * Time Complexity: O(m) where m = number of moves from square
     * @param square - 0x88 square index
     * @return Vector of legal moves from that square
     */
    std::vector<Move> getLegalMovesFrom(int square);

    /**
     * Makes a move on the board
     * Time Complexity: O(1)
     * @param move - Move to make
     * @return true if move was legal and made
     */
    bool makeMove(const Move &move);

    /**
     * Undoes the last move
     * Time Complexity: O(1)
     */
    void undoMove();

    /**
     * Checks if given side is in check
     * Time Complexity: O(n) where n = number of opponent pieces
     * @param side - 0 for white, 1 for black
     * @return true if side is in check
     */
    bool isCheck(int side) const;

    /**
     * Checks if current position is checkmate
     * Time Complexity: O(n*m) - generates all legal moves
     * @return true if current side is checkmated
     */
    bool isCheckmate();

    /**
     * Checks if current position is stalemate
     * Time Complexity: O(n*m)
     * @return true if stalemate
     */
    bool isStalemate();

    /**
     * Checks if position is draw (50-move rule, insufficient material, etc.)
     * Time Complexity: O(n) where n = number of pieces
     * @return true if position is drawn
     */
    bool isDraw() const;

    // ==================== HELPER FUNCTIONS ====================

    /**
     * Gets piece at given square
     * Time Complexity: O(1)
     * @param square - 0x88 square index
     * @return Piece value (0 if empty, positive/negative for white/black)
     */
    int getPiece(int square) const;

    /**
     * Sets piece at given square
     * Time Complexity: O(1)
     * @param square - 0x88 square index
     * @param piece - Piece value to set
     */
    void setPiece(int square, int piece);

    /**
     * Checks if square is valid (on board)
     * Time Complexity: O(1)
     * @param square - 0x88 square index
     * @return true if square is on board
     */
    bool isValidSquare(int square) const;

    /**
     * Gets current side to move
     * @return 0 for white, 1 for black
     */
    int getSideToMove() const { return sideToMove; }

    /**
     * Gets castling rights
     * @return Castling rights bitfield
     */
    int getCastlingRights() const { return castlingRights; }

    /**
     * Gets en passant square
     * @return 0x88 square index or -1 if none
     */
    int getEnPassantSquare() const { return enPassantSquare; }

    bool enemyPawnCanCaptureEP(int epSquare);
    /**
     * Gets half move clock
     * @return Number of half moves since last capture or pawn move
     */
    int getHalfMoveClock() const
    {
        return halfMoveClock;
    }

    /**
     * Gets full move number
     * @return Current full move number
     */
    int getFullMoveNumber() const { return fullMoveNumber; }

    /**
     * Prints board to console for debugging
     * Time Complexity: O(1)
     */
    void print() const;

private:
    // ==================== MOVE GENERATION ====================

    /**
     * Generates all pseudo-legal moves (doesn't check for check)
     * Time Complexity: O(n*m)
     */

    /**
     * Generate moves for specific piece types
     */
    void generateKnightMoves(std::vector<Move> &moves, int from) const;
    void generateSlidingMoves(std::vector<Move> &moves, int from,
                              const int *directions, int numDirections) const;
    void generatePawnMoves(std::vector<Move> &moves, int from) const;
    void generateKingMoves(std::vector<Move> &moves, int from) const;
    void generateCastlingMoves(std::vector<Move> &moves, int from) const;

    /**
     * Adds promotion moves to move list
     */
    void addPromotionMoves(std::vector<Move> &moves, int from, int to,
                           int piece, int capturedPiece = 0) const;

    // ==================== CHECK DETECTION ====================

    /**
     * Checks if a square is attacked by given side
     * Time Complexity: O(n) where n = number of attacking pieces
     * @param square - 0x88 square to check
     * @param attackingSide - 0 for white, 1 for black
     * @return true if square is under attack
     */
    bool isSquareAttacked(int square, int attackingSide) const;

    /**
     * Finds the king square for given side
     * Time Complexity: O(1) - worst case O(64) but typically very fast
     * @param side - 0 for white, 1 for black
     * @return 0x88 square index of king
     */
    int findKing(int side) const;

    // ==================== MOVE EXECUTION ====================

    /**
     * Updates castling rights after a move
     * Time Complexity: O(1)
     */
    void updateCastlingRights(const Move &move);
    // Insufficient material and repetition helpers
    bool isInsufficientMaterial() const;
    bool isThreefoldRepetition() const;

private:
    // Piece list optimization: quick access to occupied squares
    std::vector<int> pieceList; // list of squares (0x88) that currently hold a piece
    int pieceIndex[128];        // -1 if square not in pieceList, else index into pieceList for O(1) removal
    void rebuildPieceList();
    void addPieceSquare(int square);
    void removePieceSquare(int square);
    void movePieceSquare(int from, int to);

public:
    /**
     * Returns a const reference to the current list of piece squares
     */
    const std::vector<int> &getPieceList() const { return pieceList; }
};

// ==================== 0x88 HELPER FUNCTIONS ====================

/**
 * Checks if square is valid (on board)
 * Time Complexity: O(1)
 */
inline bool isValid0x88(int square)
{
    return (square & 0x88) == 0;
}

/**
 * Gets file (column) of square (0-7, a-h)
 * Time Complexity: O(1)
 */
inline int fileOf(int square)
{
    return square & 7;
}

/**
 * Gets rank (row) of square (0-7, 1-8)
 * Time Complexity: O(1)
 */
inline int rankOf(int square)
{
    return square >> 4;
}

/**
 * Makes 0x88 square from file and rank
 * Time Complexity: O(1)
 */
inline int makeSquare(int file, int rank)
{
    return (rank << 4) | file;
}

/**
 * Converts 0x88 square to standard 0-63 representation
 * Time Complexity: O(1)
 */
inline int from0x88(int square88)
{
    return ((square88 >> 4) << 3) | (square88 & 7);
}

/**
 * Converts 0-63 square to 0x88 representation
 * Time Complexity: O(1)
 */
inline int to0x88(int square64)
{
    return ((square64 >> 3) << 4) | (square64 & 7);
}

/**
 * Converts 0x88 square to algebraic notation (e.g., "e4")
 * Time Complexity: O(1)
 */
std::string squareToAlgebraic(int square88);

/**
 * Converts algebraic notation to 0x88 square (e.g., "e4" -> 0x34)
 * Time Complexity: O(1)
 */
int algebraicToSquare(const std::string &algebraic);

/**
 * Gets piece character for display
 * Time Complexity: O(1)
 */
char pieceToChar(int piece);

/**
 * Gets piece from character
 * Time Complexity: O(1)
 */
int charToPiece(char c);

#endif // BOARD_H
