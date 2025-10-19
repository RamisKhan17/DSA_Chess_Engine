#include "include/Board.h"
#include <iostream>

void debugPawnAttacks() {
    Board board;
    
    std::cout << "=== Detailed Pawn Attack Debug ===\n\n";
    
    // Test case: White pawn at d3, black king at c2
    // The white pawn should be able to attack the black king
    board.setFEN("8/8/8/8/8/3P4/2k5/8 w - - 0 1");
    
    std::cout << "Position: White pawn at d3, Black king at c2\n";
    board.print();
    
    // Let's manually check what squares the pawn attack detection is looking at
    int targetSquare = 0x12; // c2 (black king position)
    int attackingSide = 0;   // white
    
    std::cout << "Target square (c2): 0x" << std::hex << targetSquare << std::dec << "\n";
    
    // Current logic
    int pawnDirection = (attackingSide == 0) ? -16 : 16;
    int pawnAttacks[2] = {pawnDirection - 1, pawnDirection + 1};
    
    std::cout << "Pawn direction: " << pawnDirection << "\n";
    std::cout << "Looking for white pawns at squares:\n";
    for (int i = 0; i < 2; i++) {
        int from = targetSquare + pawnAttacks[i];
        std::cout << "  Square " << i << ": 0x" << std::hex << from << std::dec;
        if (from & 0x88) {
            std::cout << " (off board)";
        } else {
            int piece = board.getPiece(from);
            std::cout << " (file " << (from & 7) << ", rank " << (from >> 4) << ") piece: " << piece;
            if (piece == 1) std::cout << " (WHITE_PAWN)";
        }
        std::cout << "\n";
    }
    
    // Now let's see what the actual white pawn position is
    int whitePawnSquare = 0x23; // d3
    std::cout << "White pawn at d3: 0x" << std::hex << whitePawnSquare << std::dec;
    std::cout << " (file " << (whitePawnSquare & 7) << ", rank " << (whitePawnSquare >> 4) << ")\n";
    
    // Calculate what squares the white pawn at d3 can attack
    int whitePawnAttacks[2] = {whitePawnSquare + 15, whitePawnSquare + 17}; // diagonal forward
    std::cout << "White pawn at d3 can attack squares:\n";
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
    
    std::cout << "\n";
}

int main() {
    debugPawnAttacks();
    return 0;
}
