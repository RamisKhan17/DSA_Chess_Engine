#include "include/Board.h"
#include "include/GUI.h"
#include <iostream>

/**
 * Chess Engine - DSA Project
 * 
 * This is the main entry point for the chess engine application.
 * It initializes the board and GUI, then runs the main game loop.
 * 
 * Features:
 * - 0x88 board representation for efficient move generation
 * - Complete move generation including special moves (castling, en passant, promotion)
 * - Legal move filtering with check detection
 * - SFML-based graphical user interface
 * - Drag and drop piece movement
 * - Move history and game status display
 */

int main() {
    std::cout << "Chess Engine - DSA Project\n";
    std::cout << "==========================\n\n";
    std::cout << "Initializing chess engine...\n";
    
    try {
        // Create board and set starting position
        Board board;
        // board.setStartingPosition();
        board.setFEN("k7/8/8/8/8/8/1Q6/K7 b - - 0 1");
      
        std::cout << "Board initialized successfully!\n";
        std::cout << "Starting position:\n";
        board.print();
        
        // Test move generation
        std::vector<Move> legalMoves = board.generateLegalMoves();
        std::cout << "\nLegal moves from starting position: " << legalMoves.size() << "\n";
        std::cout << "Expected: 20 moves\n\n";
        
        if (legalMoves.size() != 20) {
            std::cout << "Warning: Move generation may have issues!\n";
        }
        
        std::cout << "Starting GUI...\n\n";
        std::cout << "Controls:\n";
        std::cout << "  - Click piece to select\n";
        std::cout << "  - Click destination to move\n";
        std::cout << "  - Drag and drop pieces\n";
        std::cout << "  - Press U to undo\n";
        std::cout << "  - Press R to reset\n\n";
        
        // Create and run GUI
        ChessGUI gui(board);
        gui.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nThank you for using Chess Engine!\n";
    return 0;
}
