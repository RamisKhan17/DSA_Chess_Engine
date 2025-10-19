 #include "include/Board.h"
#include <iostream>

int main() {
    Board board;
    board.setFEN("8/8/8/8/3k4/3P4/8/8 w - - 0 1");
    
    std::cout << "Position: White pawn at d3, Black king at c4\n";
    board.print();
    
    // Target square is c4 (0x34)
    int targetSquare = 0x34; // c4
    int attackingSide = 0;   // white
    
    std::cout << "Target square (c4): 0x" << std::hex << targetSquare << std::dec << "\n";
    
    // Current logic for white pawns
    int pawnDirection = -16; // look below for white pawns
    int pawnAttacks[2] = {pawnDirection - 1, pawnDirection + 1};
    
    std::cout << "Looking for white pawns at squares:\n";
    for (int i = 0; i < 2; i++) {
        int from = targetSquare + pawnAttacks[i];
        std::cout << "  Square " << i << ": 0x" << std::hex << from << std::dec;
        if (from & 0x88) {
            std::cout << " (off board)";
        } else {
            int piece = board.getPiece(from);
            std::cout << " (file " << (from & 7) << ", rank " << (from >> 4) << ") piece: " << piece;
            if (piece == 1) std::cout << " (WHITE_PAWN FOUND!)";
        }
        std::cout << "\n";
    }
    
    // The white pawn is at d3 (0x23)
    std::cout << "White pawn is at d3 (0x" << std::hex << 0x23 << std::dec << ")\n";
    
    // What squares can the white pawn at d3 attack?
    int whitePawnSquare = 0x23; // d3
    int whitePawnAttacks[2] = {whitePawnSquare + 15, whitePawnSquare + 17}; // diagonal forward
    std::cout << "White pawn at d3 attacks:\n";
    for (int i = 0; i < 2; i++) {
        int attackSquare = whitePawnAttacks[i];
        std::cout << "  Attack " << i << ": 0x" << std::hex << attackSquare << std::dec;
        if (attackSquare & 0x88) {
            std::cout << " (off board)";
        } else {
            std::cout << " (file " << (attackSquare & 7) << ", rank " << (attackSquare >> 4) << ")";
            if (attackSquare == targetSquare) {
                std::cout << " *** TARGET SQUARE ***";
            }
        }
        std::cout << "\n";
    }
    
    return 0;
}
