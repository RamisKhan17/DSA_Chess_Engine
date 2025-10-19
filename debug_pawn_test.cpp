#include "include/Board.h"
#include <iostream>
#include <cassert>

void testPawnAttacks() {
    Board board;
    
    std::cout << "=== Testing Pawn Attack Detection ===\n\n";
    
    // Test 1: White pawn attacking black king (should work)
    board.setFEN("8/8/8/8/8/3P4/2k5/8 w - - 0 1");
    std::cout << "Test 1: White pawn at d3 attacking black king at c2\n";
    board.print();
    bool blackInCheck = board.isCheck(1);
    std::cout << "Black king in check: " << (blackInCheck ? "YES" : "NO") << "\n";
    std::cout << "Expected: YES (white pawn can attack black king diagonally forward)\n\n";
    
    // Test 2: Black pawn attacking white king (should work)
    board.setFEN("8/2K5/3p4/8/8/8/8/8 b - - 0 1");
    std::cout << "Test 2: Black pawn at d6 attacking white king at c7\n";
    board.print();
    bool whiteInCheck = board.isCheck(0);
    std::cout << "White king in check: " << (whiteInCheck ? "YES" : "NO") << "\n";
    std::cout << "Expected: YES (black pawn can attack white king diagonally forward)\n\n";
    
    // Test 3: White pawn behind black king (should NOT work)
    board.setFEN("8/8/8/8/8/8/2k5/3P4 w - - 0 1");
    std::cout << "Test 3: White pawn at d1 behind black king at c2\n";
    board.print();
    bool blackInCheck2 = board.isCheck(1);
    std::cout << "Black king in check: " << (blackInCheck2 ? "YES" : "NO") << "\n";
    std::cout << "Expected: NO (white pawn cannot attack backward)\n\n";
    
    // Test 4: Black pawn behind white king (should NOT work)
    board.setFEN("3p4/2K5/8/8/8/8/8/8 b - - 0 1");
    std::cout << "Test 4: Black pawn at d8 behind white king at c7\n";
    board.print();
    bool whiteInCheck2 = board.isCheck(0);
    std::cout << "White king in check: " << (whiteInCheck2 ? "YES" : "NO") << "\n";
    std::cout << "Expected: NO (black pawn cannot attack backward)\n\n";
    
    // Test 5: White pawn to the side of black king (should NOT work)
    board.setFEN("8/8/8/8/8/8/2k5/2P5 w - - 0 1");
    std::cout << "Test 5: White pawn at c1 to the side of black king at c2\n";
    board.print();
    bool blackInCheck3 = board.isCheck(1);
    std::cout << "Black king in check: " << (blackInCheck3 ? "YES" : "NO") << "\n";
    std::cout << "Expected: NO (white pawn cannot attack straight forward)\n\n";
    
    // Test 6: Black pawn to the side of white king (should NOT work)
    board.setFEN("2p5/2K5/8/8/8/8/8/8 b - - 0 1");
    std::cout << "Test 6: Black pawn at c8 to the side of white king at c7\n";
    board.print();
    bool whiteInCheck3 = board.isCheck(0);
    std::cout << "White king in check: " << (whiteInCheck3 ? "YES" : "NO") << "\n";
    std::cout << "Expected: NO (black pawn cannot attack straight forward)\n\n";
}

int main() {
    testPawnAttacks();
    return 0;
}
