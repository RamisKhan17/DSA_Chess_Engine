#include "../include/Engine.h"
#include "../include/Board.h"
#include <cmath> // For std::abs

int Engine::evaluate() {
    int score = 0;

    // Piece values in centipawns
    const int PAWN_VALUE = 100;
    const int KNIGHT_VALUE = 300;
    const int BISHOP_VALUE = 325;
    const int ROOK_VALUE = 500;
    const int QUEEN_VALUE = 900;

    // Piece-square tables (example for pawns)
    const int pawnTable[8][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {5, 5, 5, 5, 5, 5, 5, 5},
        {1, 1, 2, 3, 3, 2, 1, 1},
        {0, 0, 0, 2, 2, 0, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {1, 1, 1, 0, 0, 1, 1, 1},
        {5, 5, 5, 5, 5, 5, 5, 5},
        {0, 0, 0, 0, 0, 0, 0, 0}
    };

    // Loop through all squares on the board
    for (int sq = 0; sq < 128; sq++) {
        if (sq & 0x88) continue; // Skip invalid squares (0x88 board representation)

        int piece = board.getPiece(sq); // Get the piece on the square
        if (piece == EMPTY) continue;  // Skip empty squares

        int value = 0;
        switch (std::abs(piece)) {
            case PAWN: // Pawn
                value = PAWN_VALUE;
                score += (piece > 0 ? value : -value); // Add material value
                // Add positional value from piece-square table
                score += (piece > 0 ? pawnTable[rankOf(sq)][fileOf(sq)] : -pawnTable[7 - rankOf(sq)][fileOf(sq)]);
                break;
            case KNIGHT: // Knight
                value = KNIGHT_VALUE;
                score += (piece > 0 ? value : -value);
                break;
            case BISHOP: // Bishop
                value = BISHOP_VALUE;
                score += (piece > 0 ? value : -value);
                break;
            case ROOK: // Rook
                value = ROOK_VALUE;
                score += (piece > 0 ? value : -value);
                break;
            case QUEEN: // Queen
                value = QUEEN_VALUE;
                score += (piece > 0 ? value : -value);
                break;
            case KING: // King
                // King has no material value but consider safety separately
                break;
        }
    }

    // Adjust score based on side to move
    if (board.getSideToMove() == BLACK) score = -score;

    return score;
}