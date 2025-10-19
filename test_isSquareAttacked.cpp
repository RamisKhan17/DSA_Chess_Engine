#include "include/Board.h"
#include <iostream>

int main() {
    Board board;
    board.setFEN("8/8/8/8/2k5/3P4/8/8 w - - 0 1");
    
    std::cout << "Testing isCheck function:\n";
    
    // Test if black king is in check
    bool checkResult = board.isCheck(1); // black king in check
    std::cout << "isCheck(1): " << (checkResult ? "true" : "false") << "\n";
    std::cout << "Expected: true (white pawn at d3 should attack black king at c4)\n";
    
    return 0;
}
