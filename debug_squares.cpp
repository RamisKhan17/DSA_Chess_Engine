#include "include/Board.h"
#include <iostream>

int main() {
    Board board;
    board.setFEN("8/8/8/8/8/3P4/2k5/8 w - - 0 1");
    
    std::cout << "Checking all squares for white pawns:\n";
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = (rank << 4) | file;
            int piece = board.getPiece(square);
            if (piece == 1) { // WHITE_PAWN
                std::cout << "White pawn found at file " << file << ", rank " << rank;
                std::cout << " (square 0x" << std::hex << square << std::dec << ")\n";
            }
        }
    }
    
    std::cout << "\nChecking target squares for white pawn attacks:\n";
    int targetSquare = 0x12; // c2
    int pawnDirection = -16; // looking backward for white pawns
    int pawnAttacks[2] = {pawnDirection - 1, pawnDirection + 1};
    
    for (int i = 0; i < 2; i++) {
        int from = targetSquare + pawnAttacks[i];
        std::cout << "Checking square 0x" << std::hex << from << std::dec;
        if (from & 0x88) {
            std::cout << " (off board)\n";
        } else {
            int piece = board.getPiece(from);
            std::cout << " piece: " << piece;
            if (piece == 1) std::cout << " (WHITE_PAWN FOUND!)";
            std::cout << "\n";
        }
    }
    
    return 0;
}
