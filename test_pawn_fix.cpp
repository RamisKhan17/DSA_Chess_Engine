#include "include/Board.h"
#include <iostream>
#include <cassert>

int main() {
    Board board;
    
    // Test case: Set up a position where a white pawn should attack a black king
    // but the black king should NOT be able to be captured by the pawn
    board.setFEN("8/8/8/8/8/8/3P4/3k4 w - - 0 1");
    
    std::cout << "Testing pawn attack detection fix...\n";
    board.print();
    
    // The white pawn at d2 should be able to attack d3, but NOT capture the black king at d1
    // because pawns can only capture diagonally forward, not backward
    
    // Check if black king is in check (should be false - pawns can't attack backward)
    bool blackInCheck = board.isCheck(1); // 1 = black
    std::cout << "Black king in check: " << (blackInCheck ? "YES" : "NO") << "\n";
    
    // This should be false - pawns can't attack backward
    assert(!blackInCheck);
    
    // Test the reverse: black pawn attacking white king
    board.setFEN("3K4/3p4/8/8/8/8/8/8 b - - 0 1");
    board.print();
    
    bool whiteInCheck = board.isCheck(0); // 0 = white
    std::cout << "White king in check: " << (whiteInCheck ? "YES" : "NO") << "\n";
    
    // This should also be false - pawns can't attack backward
    assert(!whiteInCheck);
    
    // Test legitimate pawn attacks
    board.setFEN("8/8/8/8/8/3P4/2k5/8 w - - 0 1");
    board.print();
    
    bool blackInCheck2 = board.isCheck(1);
    std::cout << "Black king in check (legitimate): " << (blackInCheck2 ? "YES" : "NO") << "\n";
    
    // This should be true - white pawn at d3 can attack black king at c2
    assert(blackInCheck2);
    
    std::cout << "All tests passed! Pawn attack detection is working correctly.\n";
    return 0;
}
