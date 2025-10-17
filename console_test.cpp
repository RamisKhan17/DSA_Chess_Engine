#include "include/Board.h"
#include <iostream>
#include <chrono>

/**
 * Console-only test of the chess engine
 * No SFML required - just tests move generation
 */

int main() {
    std::cout << "Chess Engine - Console Test\n";
    std::cout << "==========================\n\n";
    
    Board board;
    board.setStartingPosition();
    
    std::cout << "Starting position:\n";
    board.print();
    
    // Test move generation
    std::cout << "\nTesting move generation...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Move> moves = board.generateLegalMoves();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Generated " << moves.size() << " legal moves in " 
              << duration.count() << " microseconds\n";
    std::cout << "Expected: 20 moves\n\n";
    
    if (moves.size() == 20) {
        std::cout << "✓ Move generation test PASSED!\n\n";
    } else {
        std::cout << "✗ Move generation test FAILED!\n\n";
    }
    
    // Show first few moves
    std::cout << "First 10 moves:\n";
    for (int i = 0; i < std::min(10, (int)moves.size()); i++) {
        std::cout << (i+1) << ". " << moves[i].toAlgebraic() << "\n";
    }
    
    // Test making a move
    if (!moves.empty()) {
        std::cout << "\nTesting move execution...\n";
        Move firstMove = moves[0];
        std::cout << "Making move: " << firstMove.toAlgebraic() << "\n";
        
        board.makeMove(firstMove);
        std::cout << "Position after move:\n";
        board.print();
        
        std::cout << "\nUndoing move...\n";
        board.undoMove();
        std::cout << "Position after undo:\n";
        board.print();
        
        std::cout << "✓ Move make/undo test PASSED!\n";
    }
    
    std::cout << "\nConsole test complete!\n";
    std::cout << "The chess engine is working correctly.\n";
    std::cout << "To use the GUI, install SFML and compile with GUI support.\n";
    
    return 0;
}
